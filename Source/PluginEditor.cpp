#include "PluginEditor.h"

namespace
{
    const Colour kRed   (0xffe1062c);
    const Colour kRed2  (0xffff3b57);
    const Colour kInk   (0xff050506);

    std::function<String (double)> fmtFor (const String& id)
    {
        auto pct  = [] (double v) { return String ((int) std::round (v * 100.0)) + "%"; };
        auto secs = [] (double v) { return v < 1.0 ? String ((int) std::round (v * 1000.0)) + "ms" : String (v, 2) + "s"; };
        if (id == "cut")  return [] (double v) { return v >= 1000.0 ? String (v / 1000.0, 1) + "k" : String ((int) v) + "Hz"; };
        if (id == "det" || id == "vib") return [] (double v) { return String ((int) std::round (v)) + " c"; };
        if (id == "nz")   return [] (double v) { return String ((int) std::round (v / 0.4 * 100.0)) + "%"; };
        if (id == "res")  return [] (double v) { return String (v, 1); };
        if (id == "fe")   return [] (double v) { return String (v, 1) + " oct"; };
        if (id == "A" || id == "D" || id == "R" || id == "fd" || id == "glide") return secs;
        if (id == "ch_rate" || id == "tr_rate") return [] (double v) { return String (v, 1) + "Hz"; };
        if (id == "dl_time") return [] (double v) { return String ((int) std::round (v * 1000.0)) + "ms"; };
        if (id == "rv_size") return [] (double v) { return String (v, 1) + "s"; };
        return pct;
    }
}

//==============================================================================
DaluLook::DaluLook()
{
    setColour (ComboBox::textColourId, Colours::white);
    setColour (ComboBox::arrowColourId, Colours::white);
    setColour (Label::textColourId, Colours::white);
    setColour (Slider::textBoxTextColourId, Colour (0xffff8a9a));
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour (PopupMenu::backgroundColourId, Colour (0xff15151a));
    setColour (PopupMenu::textColourId, Colours::white);
    setColour (PopupMenu::highlightedBackgroundColourId, kRed);
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (ScrollBar::thumbColourId, kRed);
    setColour (ListBox::backgroundColourId, Colours::transparentBlack);
    setColour (TextButton::textColourOffId, Colours::white);
    setColour (TextButton::textColourOnId, Colours::white);
}

void DaluLook::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float startA, float endA, Slider&)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
    const float d = jmin (b.getWidth(), b.getHeight());
    const auto c = b.getCentre();
    const float r = d / 2.f;

    Path track; track.addCentredArc (c.x, c.y, r, r, 0.f, startA, endA, true);
    g.setColour (Colours::white.withAlpha (0.2f));
    g.strokePath (track, PathStrokeType (3.f, PathStrokeType::curved, PathStrokeType::rounded));

    const float ang = startA + pos * (endA - startA);
    Path val; val.addCentredArc (c.x, c.y, r, r, 0.f, startA, ang, true);
    g.setColour (kRed2);
    g.strokePath (val, PathStrokeType (3.f, PathStrokeType::curved, PathStrokeType::rounded));

    const float dr = r - 6.f;
    ColourGradient grad (Colour (0xff8a8a90), c.x - dr * 0.3f, c.y - dr * 0.6f, Colour (0xff050506), c.x + dr * 0.4f, c.y + dr, true);
    grad.addColour (0.4, Colour (0xff2b2b30));
    g.setGradientFill (grad);
    g.fillEllipse (c.x - dr, c.y - dr, dr * 2.f, dr * 2.f);
    g.setColour (Colours::white.withAlpha (0.35f));
    g.drawEllipse (c.x - dr, c.y - dr, dr * 2.f, dr * 2.f, 1.f);

    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.5f), c.x, c.y - dr, Colours::transparentWhite, c.x, c.y - dr * 0.1f, false));
    g.fillEllipse (c.x - dr * 0.8f, c.y - dr * 0.95f, dr * 1.6f, dr * 0.95f);

    Path p; p.addRoundedRectangle (-1.8f, -dr + 3.f, 3.6f, dr * 0.42f, 1.5f);
    g.setColour (kRed2);
    g.fillPath (p, AffineTransform::rotation (ang).translated (c.x, c.y));
}

void DaluLook::drawButtonBackground (Graphics& g, Button& btn, const Colour& bg, bool highlighted, bool down)
{
    auto b = btn.getLocalBounds().toFloat().reduced (0.5f);
    const bool dark = bg.getBrightness() < 0.2f;
    Colour top = dark ? Colour (0xff707076) : Colour (0xffff5a72);
    Colour m1  = dark ? Colour (0xff25252a) : kRed;
    Colour m2  = dark ? Colour (0xff000000) : Colour (0xffa00018);
    Colour bot = dark ? Colour (0xff2a2a2e) : Colour (0xffd4062a);
    if (down) { top = top.darker (0.3f); bot = bot.darker (0.3f); }
    else if (highlighted) { top = top.brighter (0.15f); }
    ColourGradient gr (top, 0.f, b.getY(), bot, 0.f, b.getBottom(), false);
    gr.addColour (0.48, m1); gr.addColour (0.52, m2);
    g.setGradientFill (gr);
    g.fillRoundedRectangle (b, 8.f);
    g.setColour (Colours::white.withAlpha (0.5f));
    g.drawRoundedRectangle (b, 8.f, 1.f);
}

void DaluLook::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox&)
{
    auto b = Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f);
    ColourGradient gr (Colour (0xff666666), 0.f, 0.f, Colour (0xff2a2a2a), 0.f, (float) h, false);
    gr.addColour (0.48, Colour (0xff222222)); gr.addColour (0.52, Colour (0xff000000));
    g.setGradientFill (gr);
    g.fillRoundedRectangle (b, 7.f);
    g.setColour (Colours::white.withAlpha (0.5f));
    g.drawRoundedRectangle (b, 7.f, 1.f);
    Path arrow;
    arrow.addTriangle ((float) w - 16.f, (float) h * 0.4f, (float) w - 8.f, (float) h * 0.4f, (float) w - 12.f, (float) h * 0.65f);
    g.setColour (Colours::white);
    g.fillPath (arrow);
}

//==============================================================================
Knob::Knob (AudioProcessorValueTreeState& apvts, const String& id, const String& caption)
{
    slider.setSliderStyle (Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (Slider::TextBoxBelow, false, 64, 14);
    slider.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    slider.textFromValueFunction = fmtFor (id);
    if (auto* p = apvts.getParameter (id))
        slider.setDoubleClickReturnValue (true, (double) p->convertFrom0to1 (p->getDefaultValue()));
    label.setText (caption, dontSendNotification);
    label.setJustificationType (Justification::centred);
    label.setFont (FontOptions (10.f));
    label.setColour (Label::textColourId, Colours::white.withAlpha (0.9f));
    addAndMakeVisible (slider);
    addAndMakeVisible (label);
    att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (apvts, id, slider);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromBottom (14));
    slider.setBounds (r);
}

//==============================================================================
// Parlante de tinta
namespace
{
    constexpr float VW = 380.f, VH = 420.f;
    constexpr float WX = 160.f, WY = 252.f, WR = 104.f, TX = 300.f, TY = 92.f, TR = 50.f;
}

SpeakerComponent::SpeakerComponent (DaluProcessor& p) : proc (p)
{
    Random rnd (11);
    auto mk = [&] (std::vector<Spike>& v, int n, float maxL)
    {
        for (int i = 0; i < n; ++i)
            v.push_back ({ rnd.nextFloat() * MathConstants<float>::twoPi,
                           14.f + std::pow (rnd.nextFloat(), 1.7f) * maxL,
                           0.015f + rnd.nextFloat() * 0.05f,
                           rnd.nextFloat() * 6.28f,
                           0.7f + rnd.nextFloat() * 1.3f });
    };
    mk (spikesW, 64, 120.f);
    mk (spikesT, 22, 46.f);
    for (int i = 0; i < 110; ++i)
        dots.push_back ({ rnd.nextFloat() * MathConstants<float>::twoPi,
                          1.08f + std::pow (rnd.nextFloat(), 1.3f) * 1.0f,
                          1.f + rnd.nextFloat() * 5.5f, rnd.nextFloat() * 6.28f, rnd.nextFloat() < 0.12f });
    startTimerHz (30);
}

void SpeakerComponent::timerCallback()
{
    const float dt = 1.f / 30.f;
    t += dt;
    const float tl = jlimit (0.f, 1.f, proc.outLevel.load() * 3.2f);
    const float tb = std::pow (jlimit (0.f, 1.f, proc.outBass.load() * 5.f), 1.2f);
    const float th = jlimit (0.f, 1.f, proc.outHigh.load() * 8.f);
    const float idle = 0.03f * (1.f + std::sin (t * 1.6f));
    lvl  += (jmax (tl, idle) - lvl) * 0.35f;
    bass += (jmax (tb, idle) - bass) * 0.45f;
    high += (th - high) * 0.35f;

    if (proc.noteKick.exchange (0.f) > 0.f) { kick = jmin (1.f, kick + 0.8f); ripples.push_back ({ WR * 0.5f, 0.8f }); }
    kick *= 0.82f;
    if (bass > prevBass + 0.14f && bass > 0.25f) ripples.push_back ({ WR * 0.5f, 0.45f });
    prevBass = bass;

    for (auto& r : ripples) { r.r += (110.f + bass * 70.f) * dt; r.alpha -= dt * 0.9f; }
    ripples.erase (std::remove_if (ripples.begin(), ripples.end(),
                   [] (const Ripple& r) { return r.alpha <= 0.f || r.r > WR * 1.9f; }), ripples.end());
    repaint();
}

void SpeakerComponent::drawSpike (Graphics& g, float cx, float cy, float R, float a, float len, float w)
{
    const float ca = std::cos (a), sa = std::sin (a);
    Path p;
    p.startNewSubPath (cx + std::cos (a - w) * R, cy + std::sin (a - w) * R);
    p.quadraticTo (cx + ca * (R + len * 0.3f), cy + sa * (R + len * 0.3f), cx + ca * (R + len), cy + sa * (R + len));
    p.quadraticTo (cx + ca * (R + len * 0.3f), cy + sa * (R + len * 0.3f), cx + std::cos (a + w) * R, cy + std::sin (a + w) * R);
    p.closeSubPath();
    g.fillPath (p);
}

void SpeakerComponent::drawDriver (Graphics& g, float cx, float cy, float R, float exc, float cap)
{
    g.setColour (kInk);
    g.fillEllipse (cx - R, cy - R, R * 2.f, R * 2.f);

    const float sr = R * 0.88f;
    g.setGradientFill (ColourGradient (Colours::white, cx - R * 0.3f, cy - R * 0.35f, Colour (0xff6e6e76), cx + R * 0.6f, cy + R * 0.6f, true));
    g.fillEllipse (cx - sr, cy - sr, sr * 2.f, sr * 2.f);

    g.setColour (kInk);
    g.drawEllipse (cx - R * 0.76f, cy - R * 0.76f, R * 1.52f, R * 1.52f, R * 0.06f);

    const float cr = R * 0.72f * (1.f + exc * 0.05f);
    ColourGradient cone (Colour (0xfff4f4f6), cx - R * 0.12f * (1.f + exc), cy - R * 0.15f, Colour (0xff33333a), cx + cr, cy + cr, true);
    cone.addColour (0.55, Colour (0xffb5b5bc));
    g.setGradientFill (cone);
    g.fillEllipse (cx - cr, cy - cr, cr * 2.f, cr * 2.f);

    for (float r = R * 0.28f; r < cr; r += R * 0.1f)
    {
        const float rr = r * (1.f + exc * 0.02f);
        g.setColour (Colours::black.withAlpha (0.2f + bass * 0.2f));
        g.drawEllipse (cx - rr, cy - rr, rr * 2.f, rr * 2.f, 1.2f);
    }

    const float cp = cap * (1.f + exc * 0.25f);
    ColourGradient capG (Colour (0xffff8696), cx - cp * 0.3f, cy - cp * 0.35f, Colour (0xff2a0007), cx + cp, cy + cp, true);
    capG.addColour (0.45, kRed);
    g.setGradientFill (capG);
    g.fillEllipse (cx - cp, cy - cp, cp * 2.f, cp * 2.f);
    g.setColour (kInk);
    g.drawEllipse (cx - cp, cy - cp, cp * 2.f, cp * 2.f, cp * 0.18f);
    g.setColour (Colours::white.withAlpha (0.55f));
    g.fillEllipse (cx - cp * 0.75f, cy - cp * 0.62f, cp * 1.0f, cp * 0.44f);

    // brillo aero
    g.saveState();
    Path clip; clip.addEllipse (cx - R * 0.9f, cy - R * 0.9f, R * 1.8f, R * 1.8f);
    g.reduceClipRegion (clip, AffineTransform());
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.5f), cx, cy - R, Colours::transparentWhite, cx, cy, false));
    g.fillEllipse (cx - R * 0.85f, cy - R * 0.92f, R * 1.7f, R * 0.84f);
    g.restoreState();
}

void SpeakerComponent::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.saveState();

    const float s = jmin (b.getWidth() / VW, b.getHeight() / VH);
    const float ox = (b.getWidth() - VW * s) * 0.5f, oy = (b.getHeight() - VH * s) * 0.5f;
    g.addTransform (AffineTransform::scale (s).translated (ox, oy));

    const float exc = jlimit (0.f, 1.3f, bass * 0.9f + kick * 0.7f);

    // halo suave para que la tinta se lea sobre el fondo rojo
    g.setGradientFill (ColourGradient (Colour (0xffff465f).withAlpha (0.2f + lvl * 0.2f), WX, WY,
                                       Colour (0xffff465f).withAlpha (0.f), WX + 260.f, WY, true));
    g.fillRect (0.f, 0.f, VW, VH);

    g.setColour (kInk);
    for (auto& sp : spikesW)
    {
        const float k = 1.f + bass * 0.9f * (0.5f + 0.5f * std::sin (t * 3.f * sp.speed + sp.phase)) + kick * 0.5f;
        drawSpike (g, WX, WY, WR * 0.95f, sp.a, sp.len * k, sp.w);
    }
    for (auto& sp : spikesT)
        drawSpike (g, TX, TY, TR * 0.95f, sp.a, sp.len * (1.f + high * 1.2f + kick * 0.2f), sp.w);

    for (auto& d : dots)
    {
        const float k = 1.f + bass * 0.35f * std::sin (t * 4.f + d.phase);
        const float x = WX + std::cos (d.a) * WR * d.d * k, y = WY + std::sin (d.a) * WR * d.d * k;
        const float r = d.r * (1.f + lvl * 0.5f);
        g.setColour (d.red ? kRed : kInk);
        g.fillEllipse (x - r, y - r, r * 2.f, r * 2.f);
    }

    // gotas
    const float dripA[5] = { 1.15f, 1.7f, 2.1f, 0.75f, 2.55f }, dripL[5] = { 55.f, 100.f, 42.f, 36.f, 66.f };
    g.setColour (kInk);
    for (int i = 0; i < 5; ++i)
    {
        const float x = WX + std::cos (dripA[i]) * WR * 0.95f, y = WY + std::sin (dripA[i]) * WR * 0.95f;
        const float l = dripL[i] * (1.f + bass * 0.5f);
        const float x2 = x + std::cos (dripA[i]) * l, y2 = y + std::sin (dripA[i]) * l;
        g.drawLine (x, y, x2, y2, 4.f - (float) i * 0.3f);
        g.fillEllipse (x2 - 3.4f, y2 - 3.4f, 6.8f, 6.8f);
    }

    drawDriver (g, WX, WY, WR, exc, 24.f);
    drawDriver (g, TX, TY, TR, jlimit (0.f, 1.2f, high * 1.6f + kick * 0.4f), 11.f);

    for (auto& r : ripples)
    {
        g.setColour (kRed.withAlpha (jlimit (0.f, 1.f, r.alpha)));
        g.drawEllipse (WX - r.r, WY - r.r, r.r * 2.f, r.r * 2.f, 3.f);
    }
    g.restoreState();
}

//==============================================================================
void DaluEditor::PresetModel::paintListBoxItem (int row, Graphics& g, int w, int h, bool)
{
    if (row < 0 || row >= (int) rows.size()) return;
    const auto& p = getPresets()[(size_t) rows[(size_t) row]];
    const bool on = rows[(size_t) row] == current;
    auto r = Rectangle<float> (1.f, 1.f, (float) w - 2.f, (float) h - 2.f);

    ColourGradient gr = on ? ColourGradient (Colour (0xffff4a64), 0.f, 0.f, Colour (0xffd1052a), 0.f, (float) h, false)
                           : ColourGradient (Colours::white, 0.f, 0.f, Colour (0xffdcdce2), 0.f, (float) h, false);
    g.setGradientFill (gr);
    g.fillRoundedRectangle (r, 3.f);
    g.setColour (on ? kInk : kRed);
    g.fillRect (1, 1, 4, h - 2);
    g.setColour (on ? Colours::white : Colour (0xff111111));
    g.setFont (14.f);
    g.drawText (p.name, 12, 0, w - 120, h, Justification::centredLeft);
    g.setColour (on ? Colour (0xffffd0d6) : Colour (0xffa00018));
    g.setFont (10.f);
    g.drawText (String (p.category).toUpperCase(), w - 100, 0, 92, h, Justification::centredRight);
}


//==============================================================================
// Fondo Frutiger Metro (sutil): degradado rojo, anillos tipo parlante, estrellas, medio tono y curvas
Image DaluEditor::makeBackground (int w, int h)
{
    Image img (Image::ARGB, jmax (1, w), jmax (1, h), true);
    Graphics g (img);
    const float W = (float) w, H = (float) h;

    g.setGradientFill (ColourGradient (Colour (0xff86101f), W * 0.85f, H * 0.08f, Colour (0xff08030a), W * 0.85f - jmax (W, H) * 0.95f, H * 0.08f + jmax (W, H) * 0.5f, true));
    g.fillAll();
    {
        ColourGradient c2 (Colour (0xffbe0a28).withAlpha (0.28f), W * 0.05f, H * 0.95f, Colour (0xffbe0a28).withAlpha (0.f), W * 0.55f, H * 0.95f, true);
        g.setGradientFill (c2); g.fillAll();
    }

    auto halftone = [&] (float x, float y, float ww, float hh, float step, float maxR, float alpha, Colour col)
    {
        g.setColour (col.withAlpha (alpha));
        for (int j = 0; j * step < hh; ++j)
            for (int i = 0; i * step < ww; ++i)
            {
                const float px = x + i * step + ((j % 2) ? step * 0.5f : 0.f), py = y + j * step;
                const float d = std::hypot (i * step / ww, j * step / hh);
                const float r = maxR * jmax (0.f, 1.f - d * 0.85f);
                if (r > 0.4f) g.fillEllipse (px - r, py - r, r * 2.f, r * 2.f);
            }
    };
    halftone (W * 0.04f, H * 0.28f, 300.f, 230.f, 9.f, 3.2f, 0.22f, Colour (0xffff6b7d));
    halftone (W * 0.58f, H * 0.5f, 260.f, 200.f, 9.f, 3.f, 0.12f, Colour (0xffff9aa8));

    auto swoosh = [&] (Colour col, float alpha, float lw, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
    {
        Path p; p.startNewSubPath (x0, y0); p.cubicTo (x1, y1, x2, y2, x3, y3);
        g.setColour (col.withAlpha (alpha));
        g.strokePath (p, PathStrokeType (lw, PathStrokeType::curved, PathStrokeType::rounded));
    };
    swoosh (Colours::black, 0.45f, 7.f, -20.f, H * 0.66f, W * 0.2f, H * 0.5f, W * 0.34f, H * 0.95f, W * 0.62f, H * 0.82f);
    swoosh (Colours::black, 0.30f, 2.5f, -20.f, H * 0.70f, W * 0.22f, H * 0.58f, W * 0.36f, H * 1.0f, W * 0.66f, H * 0.88f);
    swoosh (Colour (0xffff5a72), 0.22f, 2.f, -20.f, H * 0.62f, W * 0.18f, H * 0.46f, W * 0.3f, H * 0.86f, W * 0.58f, H * 0.76f);
    swoosh (Colours::black, 0.35f, 4.f, W * 0.5f, -10.f, W * 0.62f, H * 0.12f, W * 0.7f, H * 0.02f, W * 0.82f, H * 0.12f);

    auto ring = [&] (float x, float y, float r, Colour col, float lw, float alpha)
    {
        g.setColour (col.withAlpha (alpha));
        g.drawEllipse (x - r, y - r, r * 2.f, r * 2.f, lw);
    };
    ring (W * 0.84f, H * 0.14f, 92.f, Colours::black, 14.f, 0.5f);
    ring (W * 0.84f, H * 0.14f, 112.f, Colours::white, 2.f, 0.28f);
    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillEllipse (W * 0.84f - 42.f, H * 0.14f - 42.f, 84.f, 84.f);
    ring (W * 0.92f, H * 0.3f, 66.f, Colours::white, 2.f, 0.3f);
    ring (W * 0.78f, H * 0.32f, 48.f, Colours::black, 6.f, 0.4f);
    ring (W * 0.06f, H * 0.9f, 100.f, Colours::black, 12.f, 0.35f);
    ring (W * 0.06f, H * 0.9f, 74.f, Colours::white, 2.f, 0.22f);

    auto star = [&] (float cx, float cy, float R, float rot, bool fill, Colour col, float alpha)
    {
        Path p; p.addStar ({ cx, cy }, 5, R * 0.42f, R, rot);
        g.setColour (col.withAlpha (alpha));
        if (fill) g.fillPath (p); else g.strokePath (p, PathStrokeType (2.f));
    };
    star (W * 0.035f, H * 0.10f, 16.f, 0.2f, false, Colours::white, 0.75f);
    star (W * 0.075f, H * 0.17f, 10.f, -0.3f, false, Colours::white, 0.75f);
    star (W * 0.46f, H * 0.045f, 11.f, 0.1f, false, Colours::white, 0.75f);
    star (W * 0.58f, H * 0.07f, 34.f, 0.4f, true, Colours::black, 0.4f);
    star (W * 0.94f, H * 0.90f, 40.f, -0.2f, true, Colour (0xffff4a62), 0.22f);

    // brillo aero arriba
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.07f), 0.f, 0.f, Colours::transparentWhite, 0.f, H * 0.5f, false));
    g.fillEllipse (W * 0.4f - W * 0.75f, -H * 0.15f - H * 0.5f, W * 1.5f, H * 1.0f);
    return img;
}

//==============================================================================
DaluEditor::DaluEditor (DaluProcessor& p)
    : AudioProcessorEditor (&p), proc (p), speaker (p),
      keyboard (p.keyboardState, MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&look);

    // ---- titulo grunge (se dibuja una sola vez en una imagen)
    {
        titleImage = Image (Image::ARGB, 380, 96, true);
        Graphics g (titleImage);
        g.addTransform (AffineTransform::rotation (-0.05f, 190.f, 48.f));
        auto draw = [&] (const String& txt, float x, float y, float size, Colour fill, Colour outline, float shear)
        {
            g.saveState();
            g.addTransform (AffineTransform::shear (shear, 0.f).translated (-shear * y, 0.f));
            g.setFont (size);
            g.setColour (Colours::black);
            for (int i = 0; i < 8; ++i) g.drawSingleLineText (txt, (int) (x + 4 + (i % 3)), (int) (y + 4 + (i / 3)));
            g.setColour (outline);
            for (int dx = -2; dx <= 2; dx += 2)
                for (int dy = -2; dy <= 2; dy += 2)
                    g.drawSingleLineText (txt, (int) x + dx, (int) y + dy);
            g.setColour (fill);
            g.drawSingleLineText (txt, (int) x, (int) y);
            g.drawSingleLineText (txt, (int) x + 1, (int) y);
            g.restoreState();
        };
        draw ("DALU", 8.f, 76.f, 70.f, Colours::white, kRed, -0.2f);
        draw ("magic", 196.f, 74.f, 60.f, kRed2, Colours::white, -0.2f);
        Random rnd (4);
        for (int i = 0; i < 380; ++i)
            titleImage.clear ({ rnd.nextInt (380), rnd.nextInt (96), 1 + rnd.nextInt (3), 1 + rnd.nextInt (2) });
    }

    // ---- botones y presets
    addAndMakeVisible (prevBtn); addAndMakeVisible (nextBtn);
    prevBtn.setColour (TextButton::buttonColourId, Colours::black);
    nextBtn.setColour (TextButton::buttonColourId, Colours::black);
    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (1); };

    addFxBtn.setButtonText (CharPointer_UTF8 ("\xEF\xBC\x8B A\xC3\xB1" "adir efecto"));
    addFxBtn.setColour (TextButton::buttonColourId, kRed);
    addFxBtn.onClick = [this] { showFxMenu(); };
    addAndMakeVisible (addFxBtn);

    catBox.addItem ("Todos", 1);
    {
        StringArray seen;
        for (auto& pr : getPresets()) if (! seen.contains (pr.category)) { seen.add (pr.category); catBox.addItem (pr.category, seen.size() + 1); }
    }
    catBox.setSelectedId (1, dontSendNotification);
    catBox.onChange = [this] { rebuildPresetRows(); };
    addAndMakeVisible (catBox);

    model.onPick = [this] (int idx) { pickPreset (idx); };
    presetList.setRowHeight (26);
    addAndMakeVisible (presetList);

    lastPreset = proc.currentPreset.load();
    model.current = lastPreset;
    rebuildPresetRows();

    // ---- combos
    struct CDef { const char* id; const char* cap; };
    const CDef cdefs[6] = { { "w1", "Onda 1" }, { "w2", "Onda 2" }, { "o2", "Osc2 semitonos" }, { "uni", "Unison" }, { "vow", "Formante vocal" }, { "leg", "Modo (legato = slide)" } };
    for (int i = 0; i < 6; ++i)
    {
        auto& c = combos[(size_t) i];
        if (auto* cp = dynamic_cast<AudioParameterChoice*> (proc.apvts.getParameter (cdefs[i].id)))
            c.box.addItemList (cp->choices, 1);
        c.caption.setText (cdefs[i].cap, dontSendNotification);
        c.caption.setFont (FontOptions (10.f));
        c.caption.setColour (Label::textColourId, Colours::white.withAlpha (0.85f));
        addAndMakeVisible (c.box); addAndMakeVisible (c.caption);
        c.att = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, cdefs[i].id, c.box);
    }

    // ---- perillas
    auto addKnobs = [this] (std::vector<std::unique_ptr<Knob>>& dst, std::initializer_list<std::pair<const char*, const char*>> items)
    {
        for (auto& it : items)
        {
            dst.push_back (std::make_unique<Knob> (proc.apvts, it.first, it.second));
            addAndMakeVisible (*dst.back());
        }
    };
    addKnobs (oscKnobs,    { { "det", "Detune" }, { "mix", "Mix Osc2" }, { "sub", "Sub" }, { "nz", "Ruido" } });
    addKnobs (modKnobs,    { { "vib", "Vibrato" }, { "glide", "Glide" } });
    addKnobs (filterKnobs, { { "cut", "Cutoff" }, { "res", "Reso" }, { "fe", "Env Filtro" }, { "fd", "Dec Filtro" } });
    addKnobs (envKnobs,    { { "A", "Attack" }, { "D", "Decay" }, { "S", "Sustain" }, { "R", "Release" } });
    volKnob = std::make_unique<Knob> (proc.apvts, "vol", "Volumen");
    addAndMakeVisible (*volKnob);

    // ---- efectos
    struct FDef { const char* key; const char* title; std::vector<std::pair<const char*, const char*>> k; };
    const FDef fdefs[5] = {
        { "dr", "Drive",  { { "dr_amt", "Drive" } } },
        { "ch", "Chorus", { { "ch_mix", "Mix" }, { "ch_rate", "Rate" } } },
        { "tr", "Tremolo", { { "tr_depth", "Prof." }, { "tr_rate", "Rate" } } },
        { "dl", "Delay",  { { "dl_mix", "Mix" }, { "dl_time", "Tiempo" }, { "dl_fb", "Feedback" } } },
        { "rv", "Reverb", { { "rv_mix", "Mix" }, { "rv_size", "Tamano" } } } };
    for (int i = 0; i < 5; ++i)
    {
        auto& f = fx[(size_t) i];
        f.key = fdefs[i].key; f.title = fdefs[i].title;
        for (auto& kk : fdefs[i].k) { f.knobs.push_back (std::make_unique<Knob> (proc.apvts, kk.first, kk.second)); addChildComponent (*f.knobs.back()); }
        f.close.setColour (TextButton::buttonColourId, kRed);
        f.close.onClick = [this, i] { setFxOn (i, false); };
        addChildComponent (f.close);
        proc.apvts.addParameterListener (String ("on_") + f.key, this);
    }

    addAndMakeVisible (speaker);
    keyboard.setKeyWidth (34.f);
    keyboard.setAvailableRange (36, 96);
    keyboard.setLowestVisibleKey (48);
    keyboard.setColour (MidiKeyboardComponent::keyDownOverlayColourId, kRed.withAlpha (0.85f));
    keyboard.setColour (MidiKeyboardComponent::mouseOverKeyOverlayColourId, kRed.withAlpha (0.3f));
    addAndMakeVisible (keyboard);

    startTimerHz (10);
    setSize (1280, 760);   // al final: resized() necesita que todos los componentes ya existan
}

DaluEditor::~DaluEditor()
{
    for (auto& f : fx) proc.apvts.removeParameterListener (String ("on_") + f.key, this);
    setLookAndFeel (nullptr);
}

void DaluEditor::timerCallback()
{
    const int cur = proc.currentPreset.load();
    if (cur != lastPreset)
    {
        lastPreset = cur; model.current = cur;
        presetList.repaint(); repaint (lcdRect);
    }
}

void DaluEditor::rebuildPresetRows()
{
    model.rows.clear();
    const String cat = catBox.getText();
    const auto& pre = getPresets();
    for (int i = 0; i < (int) pre.size(); ++i)
        if (catBox.getSelectedId() == 1 || cat == pre[(size_t) i].category) model.rows.push_back (i);
    presetList.updateContent();
    presetList.repaint();
}

void DaluEditor::pickPreset (int idx)
{
    proc.loadPreset (idx);
    lastPreset = idx; model.current = idx;
    presetList.repaint(); repaint (lcdRect);
}

void DaluEditor::stepPreset (int dir)
{
    if (model.rows.empty()) return;
    int pos = 0;
    for (int i = 0; i < (int) model.rows.size(); ++i) if (model.rows[(size_t) i] == lastPreset) pos = i;
    const int n = (int) model.rows.size();
    pos = (pos + dir + n) % n;
    pickPreset (model.rows[(size_t) pos]);
    presetList.scrollToEnsureRowIsOnscreen (pos);
}

void DaluEditor::setFxOn (int i, bool on)
{
    if (auto* prm = proc.apvts.getParameter (String ("on_") + fx[(size_t) i].key))
    {
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (on ? 1.f : 0.f);
        prm->endChangeGesture();
    }
}

void DaluEditor::showFxMenu()
{
    PopupMenu m;
    for (int i = 0; i < 5; ++i)
        m.addItem (i + 1, String (fx[(size_t) i].title) + (fx[(size_t) i].on ? "   (activo)" : ""), true, fx[(size_t) i].on);
    Component::SafePointer<DaluEditor> safe (this);
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (addFxBtn), [safe] (int r)
    {
        if (safe != nullptr && r > 0) safe->setFxOn (r - 1, ! safe->fx[(size_t) (r - 1)].on);
    });
}

void DaluEditor::drawPanel (Graphics& g, Rectangle<int> r, const String& title, bool white)
{
    auto b = r.toFloat();
    if (white)
    {
        ColourGradient gr (Colours::white, 0.f, b.getY(), Colour (0xffeeeef2), 0.f, b.getBottom(), false);
        gr.addColour (0.48, Colour (0xfff3f3f5)); gr.addColour (0.52, Colour (0xffdcdce0));
        g.setGradientFill (gr);
    }
    else
    {
        ColourGradient gr (Colours::white.withAlpha (0.24f), 0.f, b.getY(), Colours::black.withAlpha (0.5f), 0.f, b.getBottom(), false);
        gr.addColour (0.48, Colours::white.withAlpha (0.06f)); gr.addColour (0.52, Colours::black.withAlpha (0.3f));
        g.setGradientFill (gr);
    }
    g.fillRoundedRectangle (b, 12.f);
    g.setColour (Colours::white.withAlpha (white ? 0.9f : 0.3f));
    g.drawRoundedRectangle (b.reduced (0.5f), 12.f, 1.f);

    g.setGradientFill (ColourGradient (kRed2, (float) r.getX() + 12.f, (float) r.getY() + 8.f, Colour (0xff8a0014), (float) r.getX() + 12.f, (float) r.getY() + 14.f, false));
    g.fillRoundedRectangle ((float) r.getX() + 12.f, (float) r.getY() + 8.f, 18.f, 6.f, 3.f);
    g.setColour (white ? Colour (0xff111111) : Colours::white);
    g.setFont (11.f);
    g.drawText (title.toUpperCase(), r.getX() + 36, r.getY() + 4, 220, 14, Justification::centredLeft);
}

void DaluEditor::paint (Graphics& g)
{
    g.fillAll (Colour (0xff060608));
    if (bgImage.isValid()) g.drawImageAt (bgImage, 0, 0);

    g.drawImageAt (titleImage, 10, 0);
    g.setColour (Colours::white.withAlpha (0.7f));
    g.setFont (10.f);
    g.drawText ("R&B 2000s  /  PLUGG  /  SYNTH", 16, 90, 320, 12, Justification::left);

    // LCD
    {
        auto b = lcdRect.toFloat();
        g.setGradientFill (ColourGradient (Colour (0xff1a0004), 0.f, b.getY(), Colour (0xff2a0008), 0.f, b.getBottom(), false));
        g.fillRoundedRectangle (b, 10.f);
        g.setColour (Colours::white.withAlpha (0.18f));
        g.fillRoundedRectangle (b.withHeight (b.getHeight() * 0.5f), 10.f);
        g.setColour (Colour (0xffff7a8d));
        g.setFont (10.f);
        const auto& pre = getPresets();
        const auto& cur = pre[(size_t) jlimit (0, (int) pre.size() - 1, lastPreset)];
        g.drawText (String (cur.category).toUpperCase(), lcdRect.getX() + 14, lcdRect.getY() + 6, 300, 12, Justification::left);
        g.setColour (Colour (0xffff2a4a));
        g.setFont (24.f);
        g.drawText (cur.name, lcdRect.getX() + 14, lcdRect.getY() + 20, lcdRect.getWidth() - 28, 30, Justification::left);
    }

    drawPanel (g, presetRect, "Presets", true);
    drawPanel (g, oscRect, "Oscilador");
    drawPanel (g, filtRect, "Filtro");
    drawPanel (g, envRect, "Envolvente");
    drawPanel (g, fxRect, "Efectos");
    drawPanel (g, modRect, "Mod / Master");

    for (auto& f : fx)
    {
        if (! f.on) continue;
        auto b = f.rect.toFloat();
        g.setGradientFill (ColourGradient (Colour (0xff7a0b1d).withAlpha (0.75f), 0.f, b.getY(), Colour (0xff1a0206).withAlpha (0.9f), 0.f, b.getBottom(), false));
        g.fillRoundedRectangle (b, 10.f);
        g.setColour (Colour (0xffff788c).withAlpha (0.6f));
        g.drawRoundedRectangle (b.reduced (0.5f), 10.f, 1.f);
        g.setColour (Colours::white);
        g.setFont (11.f);
        g.drawText (String (f.title).toUpperCase(), f.rect.getX() + 10, f.rect.getY() + 4, 100, 16, Justification::left);
    }
    bool any = false;
    for (auto& f : fx) any = any || f.on;
    if (! any)
    {
        g.setColour (Colours::white.withAlpha (0.7f));
        g.setFont (13.f);
        g.drawText (CharPointer_UTF8 ("Sin efectos. Toc\xC3\xA1 \"A\xC3\xB1" "adir efecto\" para sumar Drive, Chorus, Tremolo, Delay o Reverb."),
                    fxRect.getX() + 16, fxRect.getY() + 50, 700, 20, Justification::left);
    }
}

void DaluEditor::resized()
{
    presetRect = { 16, 104, 900, 184 };
    oscRect    = { 16, 296, 340, 150 };
    filtRect   = { 362, 296, 270, 150 };
    envRect    = { 638, 296, 278, 150 };
    fxRect     = { 16, 454, 900, 156 };
    lcdRect    = { 432, 22, 428, 56 };
    modRect    = { 930, 452, 334, 158 };

    prevBtn.setBounds (390, 22, 36, 56);
    nextBtn.setBounds (866, 22, 36, 56);
    catBox.setBounds (30, presetRect.getY() + 34, 160, 24);
    presetList.setBounds (204, presetRect.getY() + 30, 700, 146);

    int cx = oscRect.getX() + 10;
    for (int i = 0; i < 4; ++i)
    {
        combos[(size_t) i].caption.setBounds (cx, oscRect.getY() + 20, 78, 12);
        combos[(size_t) i].box.setBounds (cx, oscRect.getY() + 32, 76, 22);
        cx += 82;
    }
    combos[4].caption.setBounds (filtRect.getX() + 10, filtRect.getY() + 20, 120, 12);
    combos[4].box.setBounds (filtRect.getX() + 10, filtRect.getY() + 32, 100, 22);

    for (size_t i = 0; i < oscKnobs.size(); ++i)    oscKnobs[i]->setBounds (oscRect.getX() + 42 + (int) i * 64, oscRect.getY() + 56, 64, 92);
    for (size_t i = 0; i < filterKnobs.size(); ++i) filterKnobs[i]->setBounds (filtRect.getX() + 7 + (int) i * 64, filtRect.getY() + 56, 64, 92);
    for (size_t i = 0; i < envKnobs.size(); ++i)    envKnobs[i]->setBounds (envRect.getX() + 11 + (int) i * 64, envRect.getY() + 56, 64, 92);

    for (size_t i = 0; i < modKnobs.size(); ++i) modKnobs[i]->setBounds (modRect.getX() + 71 + (int) i * 64, modRect.getY() + 56, 64, 92);
    volKnob->setBounds (modRect.getX() + 71 + 2 * 64, modRect.getY() + 56, 64, 92);
    combos[5].caption.setBounds (modRect.getX() + 10, modRect.getY() + 20, 120, 12);
    combos[5].box.setBounds (modRect.getX() + 10, modRect.getY() + 32, 130, 22);

    int x = fxRect.getX() + 10;
    const int y0 = fxRect.getY() + 30;
    for (auto& f : fx)
    {
        f.on = proc.apvts.getRawParameterValue (String ("on_") + f.key)->load() > 0.5f;
        f.close.setVisible (f.on);
        for (auto& k : f.knobs) k->setVisible (f.on);
        if (! f.on) continue;
        const int w = (int) f.knobs.size() * 64 + 16;
        f.rect = { x, y0, w, 120 };
        f.close.setBounds (f.rect.getRight() - 24, f.rect.getY() + 4, 18, 16);
        for (size_t k = 0; k < f.knobs.size(); ++k)
            f.knobs[k]->setBounds (f.rect.getX() + 8 + (int) k * 64, f.rect.getY() + 22, 64, 96);
        x += w + 8;
    }
    addFxBtn.setBounds (fxRect.getRight() - 186, fxRect.getY() + 3, 174, 24);

    speaker.setBounds (930, 104, 334, 340);

    if (bgImage.isNull() || bgImage.getWidth() != getWidth() || bgImage.getHeight() != getHeight())
        bgImage = makeBackground (getWidth(), getHeight());
    keyboard.setBounds (16, 622, 1248, 120);
}
