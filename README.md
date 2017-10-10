# About Test Card

Test Card can generate a test pattern image at any resolution and display it either fullscreen or in a window. It is also possible to save the currently displayed image to a file.

The image can be used to determine if your monitor (or e.g. a TV your computer is connected to) is displaying pixel perfect image or if it does something funky to it.

# Why this repo

In 2016 I came across a 3840x2160 display and used it to extend the desktop on my laptop computer.

I noticed some colour artifacts in some places.  These later proved to be chroma subsampling (as allowed by HDMI specs).

Looking for a test pattern I came across Väinö Helminen's http://vah.dy.fi/testcard/ .

It was nice but I noticed an issue: color lines that are so useful to exhibit the effect of chroma subsampling had a spatial period of 3 pixels, while IMHO only a period of 2 would very clearly show subsampling.

So I contacted the author, changed code and forked the original git repository on https://git.gizmo.dy.fi/testcard.git (no web view, just `git clone` to access it) to share modifications.

# Extra features (compared with original)

* Test patterns clearly smudged when subsampling is enabled.
* Option to use custom font (requested in [issues](https://github.com/fidergo-stephane-gourichon/digital_video_test_card/issues))

# License

Test Card is released under the [GNU General Public License, version 2](http://www.gnu.org/licenses/gpl-2.0.html).

# Credits

* Original software by Väinö Helminen published on http://vah.dy.fi/testcard/ .
* Original git repository on https://git.gizmo.dy.fi/testcard.git (no web view, just `git clone` to access it).
