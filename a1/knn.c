#include <stdio.h>
#include <math.h>    // Need this for sqrt()
#include <stdlib.h>
#include <string.h>

#include "knn.h"



/* Print the image to standard output in the pgmformat.  
 * (Use diff -w to compare the printed output to the original image)
 */
void print_image(unsigned char *image) {
    printf("P2\n %d %d\n 255\n", WIDTH, HEIGHT);
    for (int i = 0; i < NUM_PIXELS; i++) {
        printf("%d ", image[i]);
        if ((i + 1) % WIDTH == 0) {
            printf("\n");
        }
    }
}

/* Return the label from an image filename.
 * The image filenames all have the same format:
 *    <image number>-<label>.pgm
 * The label in the file name is the digit that the image represents
 */
unsigned char get_label(char *filename) {
    // Find the dash in the filename
    char *dash_char = strstr(filename, "-");
    // Convert the number after the dash into a integer
    return (char) atoi(dash_char + 1);
}

/* ******************************************************************
 * Complete the remaining functions below
 * ******************************************************************/


/* Read a pgm image from filename, storing its pixels
 * in the array img.
 * It is recommended to use fscanf to read the values from the file. First, 
 * consume the header information.  You should declare two temporary variables
 * to hold the width and height fields. This allows you to use the format string
 * "P2 %d %d 255 "
 * 
 * To read in each pixel value, use the format string "%hhu " which will store
 * the number it reads in an an unsigned character.
 * (Note that the img array is a 1D array of length WIDTH*HEIGHT.)
 */
void load_image(char *filename, unsigned char *img) {
    // Open corresponding image file, read in header (which we will discard)
    FILE *f2 = fopen(filename, "r");
    if (f2 == NULL) {
        perror("fopen");
        exit(1);
    }
	int height;
    int width;
    
    if (fscanf(f2, "P2 %d %d 255", &width, &height) != 2){
        perror("fscanf error");
    }

    for(int i = 0; i < height * width; i++){
        if (fscanf(f2, "%hhu", &img[i]) != 1){
            perror("fscanf error");
        }
    }
    
    fclose(f2);
}


/**
 * Load a full dataset into a 2D array called dataset.
 *
 * The image files to load are given in filename where
 * each image file name is on a separate line.
 * 
 * For each image i:
 *  - read the pixels into row i (using load_image)
 *  - set the image label in labels[i] (using get_label)
 * 
 * Return number of images read.
 */
int load_dataset(char *filename, 
                 unsigned char dataset[MAX_SIZE][NUM_PIXELS],
                 unsigned char *labels) {
    // We expect the file to hold up to MAX_SIZE number of file names
    FILE *f1 = fopen(filename, "r");
    if (f1 == NULL) {
        perror("fopen");
        exit(1);
    }
    
    int count = 0;
    char temp_file[MAX_NAME];

    while (fscanf(f1, "%s", temp_file) != EOF){
        load_image(temp_file, dataset[count]);
        labels[count] = get_label(temp_file);
        count ++;
    }

    fclose(f1);
    return count;
}

/** 
 * Return the euclidean distance between the image pixels in the image
 * a and b.  (See handout for the euclidean distance function)
 */
double distance(unsigned char *a, unsigned char *b) {

    double sum = 0.0;

    for(int i = 0; i < NUM_PIXELS; i++){  
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

/**
 * Return the most frequent label of the K most similar images to "input"
 * in the dataset
 * Parameters:
 *  - input - an array of NUM_PIXELS unsigned chars containing the image to test
 *  - K - an int that determines how many training images are in the 
 *        K-most-similar set
 *  - dataset - a 2D array containing the set of training images.
 *  - labels - the array of labels that correspond to the training dataset
 *  - training_size - the number of images that are actually stored in dataset
 * 
 * Steps
 *   (1) Find the K images in dataset that have the smallest distance to input
 *         When evaluating an image to decide whether it belongs in the set of 
 *         K closest images, it will only replace an image in the set if its
 *         distance to the test image is strictly less than all of the images in 
 *         the current K closest images.
 *   (2) Count the frequencies of the labels in the K images
 *   (3) Return the most frequent label of these K images
 *         In the case of a tie, return the smallest value digit.
 */ 

int knn_predict(unsigned char *input, int K,
                unsigned char dataset[MAX_SIZE][NUM_PIXELS],
                unsigned char *labels,
                int training_size) {

    double k_arr[K];
    unsigned char k_labs[K];

    for (int i = 0; i < K; i++){
        k_arr[i] = distance(input, dataset[i]); 
        k_labs[i] = labels[i];
    }

    for (int i = K; i < training_size; i++){

        //Find index of largest element in k_arr
        int max = 0;
        for (int j = 1; j < K; j++){
            if (k_arr[max] < k_arr[j]){
                max = j;
            }
        }
        int current_distance = distance(input, dataset[i]); 
        if (k_arr[max] > current_distance){
            k_arr[max] = current_distance;
            k_labs[max] = labels[i];
        }
    }
    // //Find most common label
    int max_count = 1, most_freq = k_labs[0], index = 0;
    
    for (int i = 0; i < K; i++){
        int temp_count = 0;

        for (int j = 0; j < K; j++){
            if (k_labs[i] == k_labs[j]){
                temp_count ++;
            }
        }
        if (temp_count > max_count){
            most_freq = k_labs[i];
            max_count = temp_count;
            index = i;
        }
        else if(temp_count == max_count){
            if (k_arr[i] < k_arr[index]){
                most_freq = k_labs[i];
            }
        }

    }
    return most_freq;
}
