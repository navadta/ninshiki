<h1 align="center">
    Ninshiki OCR
    <br>
</h1>

A basic Optical Character Recognition program written in C for the third semester of EPITA.

## Build
* `git clone https://github.com/navadta/ninshiki`
* `make`
* If you want to train a neural network with the `./ocr/build/example/ocr` example or in the GUI, you will need to
generate a dataset, for this, run `./ocr/build/example/generate_dataset ./resources/dataset <image>` by
replacing image with the 3 fonts images present in the dataset folder.
  * `./ocr/build/example/generate_dataset ./resources/dataset arial.png`
  * `./ocr/build/example/generate_dataset ./resources/dataset nunito.png`
  * `./ocr/build/example/generate_dataset ./resources/dataset roboto.png`
* All of the executable demo files will be in `./ocr/build/example/`
* The GUI executable file will be in `./gui/build/`

## Demos
* Images
  * This demo will produce a grayscaled and a binarized version of the input image
  * Several images are available in the `./resources` path
  * Run `./ocr/build/example/images path image_file` (`./ocr/build/example/images ./resources snoopy.bmp`)
* Segmentation
  * This demo will produce an image with characters, words and lines underlined
  * An example text image is available in `./resources/text.bmp`
  * `./ocr/build/example/segmentation path image_file` (`./ocr/build/example/segmentation ./resources text.bmp`)
* XOR Neural Network
  * This demo will train a neural network to learn the Bitwise XOR function then it will output some results
  * `./ocr/build/example/xor_network activation_function` (`./ocr/build/example/xor_network 1`)
  * `1` to use the Sigmoid activation function and `2` for ELU (Exponential Linear Unit)
* Dataset Generation
  * This demo will generate a dataset as explained in the `Build` section
* OCR Training
  * This demo will train a neural network capable of recognizing characters

## GUI
* This demo will open a GUI with multiple functionalities to recognize the text from an image
* Run the GUI with `./gui/build/gui`

## Navadta Members:
##### - [Alan GUERET](https://github.com/alanretgue)
##### - [Theo DARONAT](https://github.com/Theo-DARONAT)
##### - [Adam THIBERT](https://github.com/Adamaq01)
##### - [Louis D'HOLLANDE](https://github.com/Krug666)

## License

[MIT](https://github.com/navadta/ninshiki/blob/master/LICENSE) © [Navadta](https://github.com/navadta)
