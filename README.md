# Arkanoid

![Alt Text](https://github.com/Parfyonator/Arkanoid/Game.gif)

This project uses **Prophecy SDK** for visualisation, the latest version of which you can find on [Twilight3D](http://www.twilight3d.com/) webpage. To run this solition in your Visual Studio you should modify your project's *Property Pages* in the following way:

- go to *Configuration Properties > C/C++ > General* and add `..\phrophecysdk\include` and `..\phrophecysdk\src\extlib\openexr\Half` to *Additional Include Directories*;
- go to *Configuration Properties > Linker > General* and add `..\phrophecysdk\lib\visual studio .net 2003` to *Additional Library Directories*;
- go to *Configuration Properties > Linker > Input* and add `prcore.lib` and `primport.lib` to *Additional Dependencies*.

It still may rise an error due to lack of library `ddraw.lib`. If so, you can try to find it in *Windows Kits* in *Program Files (x86)*. To attach this library to your project you should do the same steps as in the previous list. For me they were:

- go to *Configuration Properties > C/C++ > General* and add `C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include` to *Additional Include Directories*;
- go to *Configuration Properties > Linker > General* and add `C:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x86` to *Additional Library Directories*;
- go to *Configuration Properties > Linker > Input* and add `ddraw.lib` to *Additional Dependencies*.
