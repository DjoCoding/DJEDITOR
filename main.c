#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>

#include "./src/types.h"
#include "./src/djeditor.h"

// MAIN ENTRY POINT
int main(int argc, char **argv) {
    
    EDITOR editor = editor_begin(&editor);

    if (argc > 1) {
        size_t user_input_size = strlen(argv[1]);
        if (user_input_size > MAX_INPUT_SIZE)  { 
            fprintf(stderr, "ERROR: INPUT FILE NAME CAN NOT EXCEED %d CHARACTERS\n", MAX_INPUT_SIZE);
            editor_quit();
            return EXIT_FAILURE;
        } else { 
            editor.config.FILE_NAME = (char *)malloc(sizeof(char) * (MAX_INPUT_SIZE + 1));
            memcpy(editor.config.FILE_NAME, argv[1], sizeof(char) * user_input_size);
        }
    }        

    if (atexit(editor_quit) != 0) {
        fprintf(stderr, "FAILED TO SET EXIT FUNCTION\n");
        return EXIT_FAILURE;
    }

    // SAVE THE PRIMARY SNAPSHOT OF THE EDITOR
    editor_save_primary_snapshot(&editor);


    editor_load_file(&editor, editor.config.FILE_NAME);

    editor_render(&editor);

    int ch;

    while (editor.config.state != EXIT) {        
        // READ USER INPUT
        ch = getch();

        // HANDLE USER INPUT
        editor_handle_event(&editor, ch);

        // RENDER
        editor_render(&editor);

        // // APPLY CHANGES TO THE NCURSES WINDOW
        refresh();

        // LOOP
    }

    editor_quit();

    return EXIT_SUCCESS;
}


