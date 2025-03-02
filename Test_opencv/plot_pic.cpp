#include <iostream>
// for ploting image
#include <cairo.h>
#include <string>

const int SPEC_SHAPE[2] = {32, 32};

int main()
{
    int width = SPEC_SHAPE[1];
    int height = SPEC_SHAPE[0];
    // Create a surface without extra margins
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);
    int pixel_r = 151; 
    int pixel_g = 153; 
    int pixel_b = 0; 
    // Draw the spectrogram
    for (int x = 0; x < SPEC_SHAPE[0]; ++x) {
        for (int y = 0; y < SPEC_SHAPE[1]; ++y) {
            float r = (float)pixel_r / 255;
            float g = (float)pixel_g / 255;
            float b = (float)pixel_b / 255;
            if(x == 0 && y < 10)
                std::cout << "Pixel is " << r << " " << g << " " << b << "\n";
            cairo_set_source_rgb(cr, r, g, b);
            cairo_rectangle(cr, y, height - x - 1, 1, 1); // Directly fit the data
            cairo_fill(cr);
        }
    }
    std::string output_file_path = "image.png";
    cairo_destroy(cr);
    cairo_surface_write_to_png(surface, output_file_path.c_str());  // Save as PNG
    cairo_surface_destroy(surface);
    std::cout << "Successfully Plot " << output_file_path << std::endl;
    return 0;
}
