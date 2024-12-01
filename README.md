# Game



## Build

#### MacOS

##### Requirements

 - SDL2 Framework installed at `/Library/Frameworks/SDL2.framework`

```sh
gcc ./src/**/*.c -Iinclude -F/Library/Frameworks -framework SDL2 -rpath /Library/Frameworks -o ./build/out
```
