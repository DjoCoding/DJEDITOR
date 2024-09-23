The rendering stage in the highlighting mode needs all of these steps

the tokenizer is going to produce the tokens with in the current buffer
the highlighter is going to take those tokens and setup the color buffer
the renderer is going to render the current buffer frame based on the colors in the color buffer