#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//=============================================================================
// Constructors & Deconstructors
//=============================================================================
Image *createImage(int num_rows, int num_cols, int num_channels)
{
    Image *new_img;

    new_img = (Image*)calloc(1, sizeof(Image));

    new_img->num_rows = num_rows;
    new_img->num_cols = num_cols;
    new_img->num_pixels = num_rows * num_cols;
    new_img->num_channels = num_channels;

    new_img->val = (int**)calloc(new_img->num_pixels, sizeof(int*));
//  #pragma omp parallel for
    for(int i = 0; i < new_img->num_pixels; i++)
        new_img->val[i] = (int*)calloc(num_channels, sizeof(int));

    return new_img;
}

void freeImage(Image **img)
{
    if(*img != NULL)
    {
        Image *tmp;

        tmp = *img;

        for(int i = 0; i < tmp->num_pixels; i++)
            free(tmp->val[i]);
        free(tmp->val);

        free(tmp);

        *img = NULL;
    }
}

//=============================================================================
// Int
//=============================================================================
int getMaximumValue(Image *img, int channel)
{
    int max_val, chn_begin, chn_end;

    max_val = -1;
    
    if(channel == -1)
    {
        chn_begin = 0; chn_end = img->num_channels - 1;
    }
    else chn_begin = chn_end = channel;

    for(int i = 0; i < img->num_pixels; i++)
        for(int j = chn_begin; j <= chn_end; j++)
            if(max_val < img->val[i][j])
                max_val = img->val[i][j];

    return max_val;   
}

int getMinimumValue(Image *img, int channel)
{
    int min_val, chn_begin, chn_end;

    min_val = -1;
    
    if(channel == -1)
    {
        chn_begin = 0; chn_end = img->num_channels - 1;
    }
    else chn_begin = chn_end = channel;

    for(int i = 0; i < img->num_pixels; i++)
        for(int j = chn_begin; j <= chn_end; j++)
            if(min_val == -1 || min_val > img->val[i][j])
                min_val = img->val[i][j];

    return min_val;   
}

int getNormValue(Image *img)
{
    int max_val;

    max_val = getMaximumValue(img, -1);

    if(max_val > 65535)
        printError("getNormValue", "This code supports only 8-bit and 16-bit images!");

    if(max_val <= 255) return 255;
    else return 65535;
}

//=============================================================================
// Adição do método (original do DISF para vídeos) 
//=============================================================================
Image *loadImage(char *filepath)
{
    int num_channels, offset;
    unsigned char *data;
    Image *new_img;

    offset = 0;
    new_img = (Image *)calloc(1, sizeof(Image));

    data = stbi_load(filepath, &(new_img->num_cols), &(new_img->num_rows), &num_channels, 0);

    if (data == NULL)
        printError("loadImage", "Could not load the image: %s", filepath);

    new_img->num_pixels = new_img->num_rows * new_img->num_cols;

    // We do not work with alpha channels
    if (num_channels == 2 || num_channels == 4)
    {
        num_channels--;
        offset = 1;
    }

    new_img->num_channels = num_channels;

    new_img->val = (int **)calloc(new_img->num_pixels, sizeof(int *));

//  #pragma omp parallel for
    for (int i = 0; i < new_img->num_pixels; i++)
    {
        new_img->val[i] = (int *)calloc(new_img->num_channels, sizeof(int));

        for (int j = 0; j < new_img->num_channels; j++)
        {
            new_img->val[i][j] = data[i * (new_img->num_channels + offset) + j];
        }
    }

    stbi_image_free(data);

    return new_img;
}
