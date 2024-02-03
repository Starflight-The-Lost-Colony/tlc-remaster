Starflight: The Lost Colony

Development started in 2006 between Jon Harbour and Dave Calkins and quickly grew from there (see credits). In 2007, Jon was teaching game dev at UAT (uat.edu) and recruited a team of students to build a game demo for a job hunting trip to GDC in 2008. Students used the game while job hunting. After GDC, the game was an unplayable demo with key systems barely functional. During the next two years, there were only a few of us off campus working on it, and Steve Wirsz came on to do encounters and dialogue. During 2009-2010, we released the first complete beta and then the final release. A major, stable version was released in 2012 but gameplay was always unpolished. All the while, Steve was keeping the torch lit on his starflt.com web site.

In 2017-18, we worked on a major revision in an attempt to support multiple resolutions and fix old bugs and removed OpenGL (which was only used for planet orbit). The game was hard coded to 1024x768, either windowed or FS. At the time that was the norm. After the updates, any res can be selected in the Settings screen, but the screen is scaled and will remain 4:3. Even in a widescreen mode, the game renders to 4:3 with bars. It's a reasonable solution to the old problem. 

*We recommend running the game in 1600x1200. This is the same 4:3 aspect ratio as the original 1024x768 and will result in perfectly scaled art and GUI.*

Rendering is 100% software. During the 2018 revision, we removed AllegroGL for rendering planets as it required a second frame buffer and interfered with the rest of the game (such as hiding the mouse cursor). We came up with a software planet renderer that does a decent enough job with pixels. A 256x256 texture is used for planet orbit, and a 500x500 is generated for the planet surface. In summary, no 3D hardware is used at all.

*The new remastered edition has higher-resolution planet textures.*

The sources are compiled with Visual C++ 2010. To upgrade this to the latest Visual Studio, we would have to rebuild all of the libraries and the latest versions of libraries are probably not compatible with the game. This is a very old game. Strongly recommend not trying to update it. Updating the dev environment will introduce new bugs because the compiler binary format changed after 2013.
