# dragon-curve

The dragon curve is what's known as a space-filling curve. This means that although it's only 1-dimensional, it visits every single point in a 2-dimensional area. Every single point in that 2 dimensional area can be mapped to a point on the curve.

It's easy to make a finite iteration of the dragon curve yourself with just a strip of paper. First, fold the strip exactly in half. Fold it in exactly half again. Repeat this process until you're satisfied. Now, unfold the strip, making sure that all the creases are bent at 90 degrees. You now have a dragon curve.

## Usage

This program will generate an image of a finite iteration of the dragon curve.

```
./dragon-curve [TILE INPUT FILE] [OUTPUT FILE] [ITERATIONS]
./dragon-curve [TILE INPUT FILE] [OUTPUT FILE] [ITERATIONS] [RED (0-255)] [GREEN (0-255)] [BLUE (0-255)]
```

### Tile Input File

The tile input file is a file that specifies what each corner of the dragon curve should look like.

Example:

```
5
0 0 1 0 0
0 1 0 0 0
1 0 0 0 0
0 0 0 0 0
0 0 0 0 0
```

The first number is the size. It specifies the length and width of each tile in pixels.

After the size comes the grid. It's a boolean representation of whether a pixel should be colored or not. This grid should represent a corner that connects the top side and the left side of the tile.

A few tile files have been provided: `rounded.tile`, `squircle.tile`, and `octagonal.tile`.

### Output File

Where the image should be saved to. It will be in the PPM image format.

### Iterations

Which iteration of the dragon curve should be generated. This can be any non-negative integer.

### Color

The color in which the curve will be. If not specified, it will default to green.
