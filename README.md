<b>Starflight: The Lost Colony</b>

Development started in 2006 between Jon Harbour and Dave Calkins and quickly grew from there (see credits). During the next few years there were only a few of us working on it and Steve Wirsz came on to do encounters and dialogue. During 2009-2010, we released the first complete beta and then the first official release. A bug fix release was done in 2012 but gameplay was always unpolished. All the while, Steve was keeping the torch lit on his starflt.com web site.

<i>Recommend running the game in <b>1600x1200</b>. This is the same 4:3 aspect ratio as the original 1024x768 and will result in perfectly scaled art and GUI.</i>

In 2017-18, we worked on a major revision in an attempt to support multiple resolutions and removed OpenGL (which was only used for planets). The game was hard coded to 1024x768. In 2007, that was the standard. After the updates, any resolution can be selected in the Settings screen, but the screen is scaled and will remain 4:3. Even in widescreen mode, the game renders to 4:3 with bars. Rendering is 100% software. We came up with a planet renderer that does a decent enough job with pixels. A 256x256 texture is used for planet orbit and 500x500 for the planet surface.

<i>The new remastered edition has higher-resolution planet textures.</i>

The sources are compiled with Visual C++ 2010. To upgrade this to the latest Visual Studio, we would have to rebuild all of the libraries and the latest versions of libraries are probably not compatible with the game. This is a very old game. Strongly recommend not trying to update it. Updating the dev environment will introduce new bugs because the compiler binary format changed after 2013.
