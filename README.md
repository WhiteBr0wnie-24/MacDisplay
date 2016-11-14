# MacDisplay
~~Currently only shows a white screen~~

The example now works with several test images and every image you can imagine, as long as it is in the desired format. See the examples, if you want to display custom images.

Currently working on displaying the shell and x server output on the screen.
See http://www.nerdhut.de/ for informations, updates will be released there.


![alt text](https://66.media.tumblr.com/b293d0992fef8fa695c7a0584cfce5bf/tumblr_ob6ixowMJ81vzqzzwo1_1280.jpg "Macintosh Classic displays cat images")

Currently working on:
The display still has some distortion on the left and right hand side of the screen. I suspect this happens due to slightly off timings.

__UPDATE__
The distortion on the sides of the screen is caused by the way, the pixels are sent to the CRT display. Black pixels are treated first, leading to a quicker line if it has a lot of dark pixels in it, on the other hand, lines with a lot of white pixels take longer to render. My current idea is to use the PRU clock cycle counter to measure how long a line took to draw and wait, if it was quicker than a line made up entirely of white pixels. However, waiting 5ns longer in the section where a white pixel is being drawn increases the image quality drastically and there is almost no distortion left.
