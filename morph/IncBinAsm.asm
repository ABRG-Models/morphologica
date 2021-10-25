.CODE
// Function prototype: void IncBin()
// Function description: Includes binary data from files in the program's data
IncBin PROC
.pushsection vera_ttf, "a", @progbits
.incbin MORPH_FONTS_DIR "/ttf-bitstream-vera/Vera.ttf"
.popsection
ret // Function return instruction
IncBin ENDP

END //end of file
