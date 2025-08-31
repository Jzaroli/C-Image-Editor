/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Johann Zaroli

- All project requirements fully met? (YES or NO):
    Yes

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:
    No
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
//
    
// Adds vignette effect to image (dark corners)
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{
    int width = image.size();
    int height = image[0].size();

    // Fresh canvas
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Find the distance to the center                
            double distance = sqrt(pow((col - width / 2), 2) + pow((row - height / 2), 2));
            // cout << "\ndistance " << distance << "\n";
            
            double scaling_factor = (height - distance) / height;
            // cout << "\nscaling " << scaling_factor << "\n";

            // Set the red, green and blue color values at each pixel location
            new_image[row][col].red = red_color * scaling_factor;
            new_image[row][col].green = green_color * scaling_factor;
            new_image[row][col].blue = blue_color * scaling_factor;
        }
    }
    return new_image;
}

// Adds Clarendon effect to image (darks darker and lights lighter) by a scaling factor
vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int width = image.size();
    int height = image[0].size();

    // Fresh canvas
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Average those values
            double average_value = (red_color + green_color + blue_color) / 3;

            // If the cell is light, make it lighter
            if (average_value >= 170)
            {
                new_image[row][col].red = 255 - ((255 - red_color) * scaling_factor);
                new_image[row][col].green = 255 - ((255 - green_color) * scaling_factor);
                new_image[row][col].blue = 255 - ((255 - blue_color) * scaling_factor);
            }
            else if (average_value < 90)
            {
                new_image[row][col].red = red_color * scaling_factor;
                new_image[row][col].green = green_color * scaling_factor;
                new_image[row][col].blue = blue_color * scaling_factor;
            }
            else
            {
                new_image[row][col].red = red_color;
                new_image[row][col].green = green_color;
                new_image[row][col].blue = blue_color;
            }
        }
    }
    return new_image;
}

// Grayscale image
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{
    int width = image.size();
    int height = image[0].size();

    // Fresh canvas
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Average those values to get the grey value
            double gray_value = (red_color + green_color + blue_color) / 3;

            // Set new color values to all be our grey value
            new_image[row][col].red = gray_value;
            new_image[row][col].green = gray_value;
            new_image[row][col].blue = gray_value;
        }
    }
    return new_image;
}

// Rotates image by 90 degrees clockwise (not counter-clockwise)
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{
    int width = image.size();
    int height = image[0].size();

    // Fresh canvas
    vector<vector<Pixel>> new_image(height, vector<Pixel>(width));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Rotate the location of the r, g, b values:
            new_image[col][(width-1) - row].red = red_color;
            new_image[col][(width-1) - row].green = green_color;
            new_image[col][(width-1) - row].blue = blue_color;
        }
    }
    return new_image;
}

// Rotates image by a specified number of multiples of 90 degrees clockwise
vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)
{
    int angle = number * 90;
    if (angle % 90 != 0)
    {
        cout << "Angle must be a multiple of 90 degrees.";
    }
    else if (angle % 360 == 0)
    {
        return image;
    }
    else if (angle % 360 == 90)
    {
        vector<vector<Pixel>> new_image = process_4(image);
        return new_image;
    }
    else if (angle % 360 == 180)
    {
        vector<vector<Pixel>> new_image = process_4(process_4(image));
        return new_image;
    }
    else
    {
        vector<vector<Pixel>> new_image = process_4(process_4(process_4(image)));
        return new_image;
    }
    return image;
}

// Enlarges the image in the x and y direction
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale)
{
    int width = image.size();
    int height = image[0].size();
    
    // Calculates new dimensions based on user input
    int new_width = width * y_scale;
    int new_height = height * x_scale;
    
    // Fresh canvas
    vector<vector<Pixel>> new_image(new_width, vector<Pixel>(new_height));
    
    for (int row = 0; row < new_width; row++)
    {
        for (int col = 0; col < new_height; col++)
        {
            int row_fractional = row / y_scale;
            int col_fractional = col / x_scale;

            // Get the red, green and blue color values at each pixel location and within scale
            int red_color = image[row_fractional][col_fractional].red;
            int green_color = image[row_fractional][col_fractional].green;
            int blue_color = image[row_fractional][col_fractional].blue;

            // Set the values according to scale
            new_image[row][col].red = red_color;
            new_image[row][col].green = green_color;
            new_image[row][col].blue = blue_color;
        }
    }
    return new_image;
}

// Convert image to high contrast (black and white only)
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image)
{
    int width = image.size();
    int height = image[0].size();
    
    // Fresh canvas
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Finds gray value of pixel
            double gray_value = (red_color + green_color + blue_color) / 3;

            // Set the red, green and blue color values at each pixel location
            if (gray_value >= 255 / 2)
            {
                new_image[row][col].red = 255;
                new_image[row][col].green = 255;
                new_image[row][col].blue = 255;
            }
            else
            {
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
                new_image[row][col].blue = 0;
            }
        }
    }
    return new_image;
}

// Lightens image by a scaling factor
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor) 
{
    int width = image.size();
    int height = image[0].size();
    
    // Fresh canvas
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Set the red, green and blue color values at each pixel location
            new_image[row][col].red = (255 - ((255 - red_color) * scaling_factor));
            new_image[row][col].green = (255 - ((255 - green_color) * scaling_factor));
            new_image[row][col].blue = (255 - ((255 - blue_color) * scaling_factor));
        }
    }
    return new_image;
}

// Darkens image by a scaling factor

vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)
{
    int width = image.size();
    int height = image[0].size();
    
    // Fresh canvas
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Set the red, green and blue color values at each pixel location
            new_image[row][col].red = red_color * scaling_factor;
            new_image[row][col].green = green_color * scaling_factor;
            new_image[row][col].blue = blue_color * scaling_factor;
        }
    }
    return new_image;
}

// Converts image to only black, white, red, blue, and green
vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
{
    int width = image.size();
    int height = image[0].size();
    
    // Fresh canvas
    vector<vector<Pixel>> new_image(width, vector<Pixel>(height));
    
    for (int row = 0; row < width; row++)
    {
        for (int col = 0; col < height; col++)
        {
            // Get the red, green and blue color values at each pixel location.
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            // Get max/largest color number
            int max_color = red_color;
            
            if (green_color > max_color)
            {
                max_color = green_color;
            }
            if (blue_color > max_color)
            {
                max_color = blue_color;
            }

            // Set the red, green and blue color values at each pixel location
            if (red_color + green_color + blue_color >= 550)
            {
                new_image[row][col].red = 255;
                new_image[row][col].green = 255;
                new_image[row][col].blue = 255;
            }
            else if (red_color + green_color + blue_color <= 150)
            {
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
                new_image[row][col].blue = 0;
            }
            else if (max_color == red_color)
            {
                new_image[row][col].red = 255;
                new_image[row][col].green = 0;
                new_image[row][col].blue = 0;
            }
            else if (max_color == green_color)
            {
                new_image[row][col].red = 0;
                new_image[row][col].green = 255;
                new_image[row][col].blue = 0;
            }
            else
            {
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
                new_image[row][col].blue = 255;
            }
            
        }
    }
    return new_image;
}
    
int main()
{
    cout << "CSPB 1300 Image Processing Application" << "\n"; // Welcome Statement
    bool is_menu_active = true; // Value for while loop menu
    
    // Prompts user for relative file path / name
    string filename = "";
    cout << "Enter input BMP filename: " << "\n";
    cin >> filename;
    
    // Read in image from relative path:
    string sample_image_location = "./" + filename;
    
    // Declaration of output file
    string output_filename = "";
    
    while (is_menu_active)
    {
        string chosen_option = "";
        
        cout << "IMAGE PROCESSING MENU" << "\n";
        cout << "0) Change image (current: " + filename + ")" << "\n";
        cout << "1) Vignette " << "\n";
        cout << "2) Clarendon " << "\n";
        cout << "3) Grayscale " << "\n";
        cout << "4) Rotate 90 degrees " << "\n";
        cout << "5) Rotate multiple 90 degrees " << "\n";
        cout << "6) Enlarge " << "\n";
        cout << "7) High Contrast " << "\n";
        cout << "8) Lighten " << "\n";
        cout << "9) Darken " << "\n";
        cout << "10) Black, white, red, green, blue " << "\n";
        
        cout << "\n" << "Enter menu selection (Q to quit): " << "\n";
        cin >> chosen_option;
        
        if (chosen_option == "Q")
        {
            is_menu_active = false;
            cout << "Quitting the application..." << "\n";
        }
        else if (chosen_option == "0")
        {
            cout << "Enter new input BMP filename: " << "\n";
            cin >> filename;
            sample_image_location = "./" + filename;
        }
        else if (chosen_option == "1")
        {
            cout << "Vignette selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);

            // Call process_1 function using the input 2D vector and returns a new 2D vector
            vector<vector<Pixel>> new_image = process_1(image);
            
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully applied vignette!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "2")
        {
            cout << "Clarendon selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            double scaling_factor = 0;
            cout << "Enter scaling factor: " << "\n";
            cin >> scaling_factor;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);
            
            // Call process_2
            vector<vector<Pixel>> new_image = process_2(image, scaling_factor);

            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully applied clarendon!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "3")
        {
            cout << "Grayscale selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);
            
            // Call process_3
            vector<vector<Pixel>> new_image = process_3(image);

            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully applied grayscale!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "4")
        {
            cout << "Rotate 90 degrees selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);
            
            // Call process_4
            vector<vector<Pixel>> new_image = process_4(image);

            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully applied 90 degree rotation!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "5")
        {
            cout << "Rotate multiple 90 degrees selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            double number_rotations = 0;
            cout << "Enter number of 90 degree rotations: " << "\n";
            cin >> number_rotations;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);
            
            // Call process_5
            vector<vector<Pixel>> new_image = process_5(image, number_rotations);

            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully applied multiple 90 degree rotations!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "6")
        {
            cout << "Enlarge selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            int x_scale = 0;
            cout << "Enter X scale: " << "\n";
            cin >> x_scale;
            
            int y_scale = 0;
            cout << "Enter Y scale: " << "\n";
            cin >> y_scale;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);
            
            // Call process_6
            vector<vector<Pixel>> new_image = process_6(image, x_scale, y_scale);

            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully enlarged!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "7")
        {
            cout << "High contrast selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);

            // Call process_1 function using the input 2D vector and returns a new 2D vector
            vector<vector<Pixel>> new_image = process_7(image);
            
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully applied high contrast!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "8")
        {
            cout << "Lighten selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            double scaling_factor = 0;
            cout << "Enter scaling factor " << "\n";
            cin >> scaling_factor;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);

            // Call process_1 function using the input 2D vector and returns a new 2D vector
            vector<vector<Pixel>> new_image = process_8(image, scaling_factor);
            
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully lightened!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "9")
        {
            cout << "Darkened selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            double scaling_factor = 0;
            cout << "Enter scaling factor " << "\n";
            cin >> scaling_factor;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);

            // Call process_1 function using the input 2D vector and returns a new 2D vector
            vector<vector<Pixel>> new_image = process_9(image, scaling_factor);
            
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully darkened!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
        else if (chosen_option == "10")
        {
            cout << "Black, white, red, green, blue selected" << "\n";
            cout << "Enter output BMP filename: " << "\n";
            cin >> output_filename;
            
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(sample_image_location);

            // Call process_1 function using the input 2D vector and returns a new 2D vector
            vector<vector<Pixel>> new_image = process_10(image);
            
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool image_created = write_image(output_filename, new_image);
            
            // Validates successful creation and error
            if (image_created)
            {
                cout << "Successfully applied black, white, red, green, blue filter!" << "\n" << "\n";
            }
            else if (!image_created)
            {
                cout << "There was an error performing this operation" << "\n" ;
            }
        }
    }

    return 0;
}