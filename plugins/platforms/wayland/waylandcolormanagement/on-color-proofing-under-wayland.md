# On color proofing under Wayland

ICCv4 specification is really weird in area that is related to the white point
of the profile's color space. ICC defines two different white points:

* `cmsSigChromaticAdaptationTag` defines the color adaptation of the viewer
  in which the resulting image is viewed. For Display class profiles
  ICCv4 explicitly specifies that the user is assumed to be fully adapted
  to the display. In layman terms it means that the user does not look
  anywhere outside the of the display, hence display's white looks as
  "perfect white" for him/her.

* `cmsSigMediaWhitePointTag` defines the color tint of the paper used
  for printing. This tint will be visible to the user even **after**
  he/she is fully adapted to the environment light (as defined by
  `cmsSigChromaticAdaptationTag`). For Display class profiles
  ICCv4 explicitly specifies that `cmsSigMediaWhitePointTag` must
  be set to `D50`, which means that the user is fully adapted to
  the display.

The weird thing is that ICC's "absolute colorimetric" is not really
"absolute". It still adjusts `cmsSigChromaticAdaptationTag` between
the source and destination profiles, ignoring `cmsSigMediaWhitePointTag`
difference.

From the user perspective we have three usecases:

## Gamut preview in the display space

We basically show the image to the user as if the he/she was
fully adapted to the **paper white** of the output. I.e.
both `cmsSigMediaWhitePointTag` and `cmsSigChromaticAdaptationTag`
values are taken into account (well, internally they are **not**
taken into account since their values are encoded into AToB1 and
BToA1 pipelines).

When using Wayland we set up proofing pipelines in the following way:

1) Image -> Proofing Space [0]
    * intent: chosen by the user
    * bpc: chosen by the user

2) Proofing Space -> Monitor Space
    * intent: `IntentPerceptual`
    * adaptation state: _unused_
    * bpc: `true`

3) Wayland surface
    * intent: `render_intent_relative_bpc`

## Simulate paper white in the display space **with full adaptation**

The user might want to preview the paper white in the current
display space, while keeping the eye adapted to the display's
white. It means that the color will be adpted using
`cmsSigChromaticAdaptationTag` of the profiles, but **not**
for `cmsSigMediaWhitePointTag`.

The the image's white will look as different to the diffuse
white of the display, as different the real paper white when
looked on in the environment defined by
`cmsSigChromaticAdaptationTag`.

When using Wayland we set up proofing pipelines in the following way:

1) Image -> Proofing Space [0]
    * intent: chosen by the user
    * bpc: chosen by the user

2) Proofing Space -> Monitor Space
    * intent: `IntentAbsoluteColorimetric`
    * adaptation state: `1.0`
    * bpc: _unused_

3) Wayland surface
    * intent: `render_intent_absolute` [1]

## Simulate paper white in the display space **without adaptation**

Sometimes the user may want to look into the image without full adaptation
to the display's white point. It may be useful in the case when the user
stretches the source image to full-screen, puts the printed version on
the table in front of the display and tries to compare them.

If the Output profile for the media encodes the adaptation state of
the viewing environment, and and Display profile encodes the white
point of the display, then the resulting image on the screen will have
skewed white point, equal to the one on the printed media [2]

When using Wayland we set up proofing pipelines in the following way:

1) Image -> Proofing Space [0]
    * intent: chosen by the user
    * bpc: chosen by the user

2) Proofing Space -> Monitor Space
    * intent: `IntentAbsoluteColorimetric`
    * adaptation state: `0.0`
    * bpc: _unused_

3) Wayland surface
    * intent: `render_intent_absolute_no_adaptation`

## Footnotes

[0] - "Image -> Proofing Space" leg of the conversion is chosen by the user.
It means that the user is responsible to convert the image into the
"Proofing Space" using the selected intent right before sending to
the printing device. This step is mandatory to get correct colors in the end.

[1] - In early versions of Wayland protocol `render_intent_absolute` was implemented
to work as `render_intent_absolute_no_adaptation`. For such buggy versions of
the compositor one should use `render_intent_relative` instead. The change happened in
this patch in [kwin](https://invent.kde.org/plasma/kwin/-/merge_requests/8032)
and [wayland](https://gitlab.freedesktop.org/wayland/wayland-protocols/-/merge_requests/439/).

[2] - It is usually recommended to calibrate the display to the same white
point as the viewing environment around it. But that might be not easy to guarantee
their equality, especially in case of laptops.