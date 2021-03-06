/*
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
 * Copyright (C) 2011, 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL GOOGLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sky/engine/config.h"
#include "sky/engine/core/html/parser/HTMLTreeBuilder.h"

#include "gen/sky/core/HTMLNames.h"
#include "sky/engine/bindings/core/v8/ExceptionStatePlaceholder.h"
#include "sky/engine/core/dom/DocumentFragment.h"
#include "sky/engine/core/html/HTMLDocument.h"
#include "sky/engine/core/html/HTMLTemplateElement.h"
#include "sky/engine/core/html/parser/AtomicHTMLToken.h"
#include "sky/engine/core/html/parser/HTMLDocumentParser.h"
#include "sky/engine/core/html/parser/HTMLParserIdioms.h"
#include "sky/engine/core/html/parser/HTMLToken.h"
#include "sky/engine/core/html/parser/HTMLTokenizer.h"

namespace blink {

static TextPosition uninitializedPositionValue1()
{
    return TextPosition(OrdinalNumber::fromOneBasedInt(-1), OrdinalNumber::first());
}

HTMLTreeBuilder::HTMLTreeBuilder(HTMLDocumentParser* parser, HTMLDocument* document, bool)
    :
#if ENABLE(ASSERT)
      m_isAttached(true),
#endif
      m_tree(document)
    , m_insertionMode(HTMLMode)
    , m_originalInsertionMode(HTMLMode)
    , m_parser(parser)
    , m_scriptToProcessStartPosition(uninitializedPositionValue1())
{
    m_tree.openElements()->pushRootNode(document);
}

HTMLTreeBuilder::~HTMLTreeBuilder()
{
}

void HTMLTreeBuilder::detach()
{
#if ENABLE(ASSERT)
    // This call makes little sense in fragment mode, but for consistency
    // DocumentParser expects detach() to always be called before it's destroyed.
    m_isAttached = false;
#endif
    // HTMLConstructionSite might be on the callstack when detach() is called
    // otherwise we'd just call m_tree.clear() here instead.
    m_tree.detach();
}

HTMLTreeBuilder::FragmentParsingContext::FragmentParsingContext()
    : m_fragment(nullptr)
{
}

HTMLTreeBuilder::FragmentParsingContext::FragmentParsingContext(DocumentFragment* fragment, Element* contextElement)
    : m_fragment(fragment)
{
    ASSERT(!fragment->hasChildren());
}

HTMLTreeBuilder::FragmentParsingContext::~FragmentParsingContext()
{
}

PassRefPtr<Element> HTMLTreeBuilder::takeScriptToProcess(TextPosition& scriptStartPosition)
{
    ASSERT(m_scriptToProcess);
    ASSERT(!m_tree.hasPendingTasks());
    // Unpause ourselves, callers may pause us again when processing the script.
    // The HTML5 spec is written as though scripts are executed inside the tree
    // builder.  We pause the parser to exit the tree builder, and then resume
    // before running scripts.
    scriptStartPosition = m_scriptToProcessStartPosition;
    m_scriptToProcessStartPosition = uninitializedPositionValue1();
    return m_scriptToProcess.release();
}

void HTMLTreeBuilder::constructTree(AtomicHTMLToken* token)
{
    const HTMLToken::Type type = token->type();

    if (type == HTMLToken::Character) {
        processCharacter(token);
        return;
    }

    // Any non-character token needs to cause us to flush any pending text immediately.
    // NOTE: flush() can cause any queued tasks to execute, possibly re-entering the parser.
    m_tree.flush();

    if (type == HTMLToken::StartTag) {
        processStartTag(token);
    } else if (type == HTMLToken::EndTag) {
        processEndTag(token);
    } else if (type == HTMLToken::EndOfFile) {
        processEndOfFile(token);
    } else {
        ASSERT_NOT_REACHED();
    }

    m_tree.executeQueuedTasks();
    // We might be detached now.
}

void HTMLTreeBuilder::processStartTag(AtomicHTMLToken* token)
{
    ASSERT(token->type() == HTMLToken::StartTag);
    const AtomicString& name = token->name();

    if (name == HTMLNames::styleTag) {
        processGenericRawTextStartTag(token);
    } else if (name == HTMLNames::scriptTag) {
        processScriptStartTag(token);
    } else if (token->selfClosing()) {
        m_tree.insertSelfClosingHTMLElement(token);
    } else {
        m_tree.insertHTMLElement(token);
    }
}

void HTMLTreeBuilder::processEndTag(AtomicHTMLToken* token)
{
    ASSERT(token->type() == HTMLToken::EndTag);

    const InsertionMode mode = insertionMode();
    if (mode == HTMLMode) {
        HTMLElementStack::ElementRecord* record = m_tree.openElements()->topRecord();
        while (record->next()) {
            RefPtr<Element> element = record->element();
            if (element->hasLocalName(token->name())) {
                m_tree.openElements()->popUntilPopped(element.get());
                ASSERT(m_tree.openElements()->topNode());
                return;
            }
            record = record->next();
        }
        return;
    }

    ASSERT(mode == TextMode);
    if (token->name() == HTMLNames::scriptTag) {
        // Pause ourselves so that parsing stops until the script can be processed by the caller.
        ASSERT(m_tree.currentElement()->hasLocalName(HTMLNames::scriptTag.localName()));
        m_scriptToProcess = m_tree.currentElement();
        m_tree.openElements()->pop();
        setInsertionMode(m_originalInsertionMode);
        return;
    }
    m_tree.openElements()->pop();
    setInsertionMode(m_originalInsertionMode);
}

void HTMLTreeBuilder::processCharacter(AtomicHTMLToken* token)
{
    ASSERT(token->type() == HTMLToken::Character);
    m_tree.insertTextNode(token->characters());
}

void HTMLTreeBuilder::processEndOfFile(AtomicHTMLToken* token)
{
    ASSERT(token->type() == HTMLToken::EndOfFile);
    m_tree.processEndOfFile();
}

void HTMLTreeBuilder::processGenericRawTextStartTag(AtomicHTMLToken* token)
{
    ASSERT(token->type() == HTMLToken::StartTag);
    m_tree.insertHTMLElement(token);
    m_originalInsertionMode = m_insertionMode;
    setInsertionMode(TextMode);
}

void HTMLTreeBuilder::processScriptStartTag(AtomicHTMLToken* token)
{
    ASSERT(token->type() == HTMLToken::StartTag);
    m_tree.insertScriptElement(token);
    m_originalInsertionMode = m_insertionMode;
    TextPosition position = m_parser->textPosition();
    m_scriptToProcessStartPosition = position;
    setInsertionMode(TextMode);
}

void HTMLTreeBuilder::finished()
{
    if (isParsingFragment())
        return;
    ASSERT(m_isAttached);
    // Warning, this may detach the parser. Do not do anything else after this.
    m_tree.finishedParsing();
}

} // namespace blink
