Run the incbin binary to create the file include/verafonts.h for
visual studio and a windows build.

include/verafonts.h is version-controlled, and part of the repository,
so only needs to be regenerated if additional fonts are to be included
on a Windows machine.

To generate, do this from the base morphologica directory:

```
./path/to/incbin.exe morph/VisualFace.h -p vf_ -o include/verafonts.h
```

The ```-p vf_``` is very important!