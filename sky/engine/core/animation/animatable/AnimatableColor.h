/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SKY_ENGINE_CORE_ANIMATION_ANIMATABLE_ANIMATABLECOLOR_H_
#define SKY_ENGINE_CORE_ANIMATION_ANIMATABLE_ANIMATABLECOLOR_H_

#include "sky/engine/core/animation/animatable/AnimatableValue.h"
#include "sky/engine/platform/graphics/Color.h"

namespace blink {

class AnimatableColorImpl {
public:
    AnimatableColorImpl(float red, float green, float blue, float alpha);
    AnimatableColorImpl(Color);
    Color toColor() const;
    AnimatableColorImpl interpolateTo(const AnimatableColorImpl&, double fraction) const;
    bool operator==(const AnimatableColorImpl&) const;
    double distanceTo(const AnimatableColorImpl&) const;

private:
    float m_alpha;
    float m_red;
    float m_green;
    float m_blue;
};

// This class handles both the regular and 'visted link' colors for a given
// property. Currently it is used for all properties, even those which do not
// support a separate 'visited link' color (eg SVG properties). This is correct
// but inefficient.
class AnimatableColor final : public AnimatableValue {
public:
    static PassRefPtr<AnimatableColor> create(const AnimatableColorImpl&);
    Color color() const { return m_color.toColor(); }

protected:
    virtual PassRefPtr<AnimatableValue> interpolateTo(const AnimatableValue*, double fraction) const override;

private:
    AnimatableColor(const AnimatableColorImpl& color)
        : m_color(color)
    {
    }
    virtual AnimatableType type() const override { return TypeColor; }
    virtual bool equalTo(const AnimatableValue*) const override;
    virtual double distanceTo(const AnimatableValue*) const override;
    const AnimatableColorImpl m_color;
};

DEFINE_ANIMATABLE_VALUE_TYPE_CASTS(AnimatableColor, isColor());

} // namespace blink

#endif  // SKY_ENGINE_CORE_ANIMATION_ANIMATABLE_ANIMATABLECOLOR_H_
