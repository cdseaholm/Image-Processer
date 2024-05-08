/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Carl Seaholm

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:
    To the image itself no, I went with the standard proccesses. I tried to make the U.I. as interactive and intuitive as possible since the terminal can be confusing if the text piles up. I implemented an "assistant" that gives the messages with a number so a user can use those numbers to see where they are at in the terminal. I also implemented a menu option style to most of the sections to give it a more unionized feel. And finally, I added in a place to see all the descriptions of the effects by going to "more options," then "descriptions." From there the user can still hit "back to options," to exit, change a file, or go back to effects.
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
#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <cmath>
#include <tuple>
#include <algorithm>
#include <limits>

using namespace std;
//////////////////////

///////////////////////DEFINITIONS&VARIABLES///////////////

vector<string> jeevesDialogue() {
    vector<string> jeevesDialogue = {
        //0
        "\n\n*****************************\n*****************************\nCSPB 1300\nImage Processing Application\nWritten by Carl Seaholm.\n*****************************\n\n\n*****************************\n**********WELCOME!***********\nYou have a new message from Jeeves.\nHe says: \n\nHello! My Name is Jeeves, the Image Processing Assistant.\n\nTo begin, enter a BMP filename or press Q to quit: ",
        //1
        "Great! What a lovely photo.\n\nIf you could just tell me what effect you'd like added to the photo, I can begin processing this right away.\n\nJust type the number of the effect you'd like to use. Then hit enter: ",
        //2
        "To begin, enter a BMP filename or press Q to quit: ",
        //3
        "Make sure your file is in a BMP format",
        //4
        "Enter a valid fileName. It cannot be empty.",
        //5
        "Invalid choice. Please enter a number between 1 and ",
        //6
        "Here are some more options: ",
        //7
        "Which effect would you like to know more about?",
        //8
        "After you type your choice, press enter: ",
        //9
        "What other effect would you like to know about?",
        //10
        "Enter a BMP file to use or press Q to quit: ",
        //11
        "Great, I'll begin processing this image with process ",
        //12
        "Do you want to use the same BMP file? Or do you want to use a different one?\n\nType y for yes, q to quit, or anything else to use a new file: ",
        //13
        "The photo came out great with that effect. I also saved it into the folder for you. Here is the name: ",
        //14
        "Which effect would you like to use? Type it, then hit enter: "
        
    };
    return jeevesDialogue;
};

vector<string> options() {
   vector<string> options = {
        "Vignette",
        "Clarendon",
        "Grayscale",
        "Rotate 90 degrees clockwise",
        "Rotate multiple 90 degrees clockwise",
        "Enlarge",
        "High contrast",
        "Lighten",
        "Darken",
        "Primary colors, black, and white only",
    };
    return options;
};

vector<string> descriptions() {
    vector<string> descriptions = {
    "Adding a Vignette Effect Darkens the Corners",
    "Adding a Clarendon Effect Darkens the darker areas and lightens the light ones. Done with a scaling factor,"
    "Converting to Grayscale will remove any primary colors and convert it all to shades of Gray",
    "Converting to a high contrast will make the image only black and white",
    "Converting image to only primary colors, black and white, removes any nuance or combinations of colors. Leaving only Red, Blue, Green, Black and White",
    "Darkening the image, darkens every color in the image. Done with a scaling factor,"
    "Lightening the image, lightens every color in the image. Done with a scaling factor,"
    "Enlarges the image along the X and Y axis",
    "Rotates either Clockwise 90 degrees or Counterclockwise 90 degrees a specific number of times."
    };
    return descriptions;
};


string jeevesRes(int counter, string message) {
    string countToString = to_string(counter);
    string newMessage = "\n\n*****************************\n****Message " + countToString + " from Jeeves****\n*****************************\n\n" + message + "\n\n******END OF MESSAGE: " + countToString + "******\n\n";
    return newMessage;
}

////////////////////////END OF VARIABLES////////////////////
///////////////////////PROCESSES////////////////////////
vector<vector<Pixel>> p1(vector<vector<Pixel>> image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(height, vector<Pixel>(width));
    double maxDistance = sqrt(pow(width / 2, 2) + pow(height / 2, 2));

   for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            double distance = sqrt(pow(col - width / 2, 2) + pow(row - height / 2, 2));
            double scalingFactor = 1.0 - (distance / maxDistance);
            int newRed = pixel.red * scalingFactor;
            int newGreen = pixel.green * scalingFactor;
            int newBlue = pixel.blue * scalingFactor;
            newRed = max(0, min(newRed, 255));
            newGreen = max(0, min(newGreen, 255));
            newBlue = max(0, min(newBlue, 255));
            // Initialize Pixel members directly
            newImage[row][col].red = newRed;
            newImage[row][col].green = newGreen;
            newImage[row][col].blue = newBlue;
        }
   }
   return newImage; 
}

vector<vector<Pixel>> p2(vector<vector<Pixel>> image, double scaler) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(height, vector<Pixel>(width));

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            int red_value = pixel.red;
            int green_value = pixel.green;
            int blue_value = pixel.blue;
            int average_value = (red_value + green_value + blue_value)/3;
            if (average_value >= 170) {
                newImage[row][col].red = int(255 - (255 - red_value)*scaler);
                newImage[row][col].green = int(255 - (255 - green_value)*scaler);
                newImage[row][col].blue = int(255 - (255 - blue_value)*scaler);
            } else if (average_value < 90) {
                newImage[row][col].red = red_value*scaler;
                newImage[row][col].green = green_value*scaler;
                newImage[row][col].blue = blue_value*scaler;
            } else {
                newImage[row][col].red = red_value;
                newImage[row][col].green = green_value;
                newImage[row][col].blue = blue_value;
            };
        }
    }
    return newImage; 
}

vector<vector<Pixel>> p3(vector<vector<Pixel>> image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(height, vector<Pixel>(width));

   for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            int red_value = pixel.red;
            int green_value = pixel.green;
            int blue_value = pixel.blue;
            int gray_value = (red_value + green_value + blue_value)/3;
            newImage[row][col].red = gray_value;
            newImage[row][col].green = gray_value;
            newImage[row][col].blue = gray_value;
        }
   }
   return newImage; 
}

vector<vector<Pixel>> p4(vector<vector<Pixel>> image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(width, vector<Pixel>(height));

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            newImage[col][height - 1 - row] = pixel;
        }
    }
    return newImage;
}

vector<vector<Pixel>> p5(vector<vector<Pixel>> image, int firstInt) {
    int angle = firstInt*90;
    if (angle%360 == 0) {
        return image;
    } else if (angle%360 == 90) {
        return p4(image);
    } else if (angle%360 == 180) {
        return p4(p4(image));
    } else {
        return p4(p4(p4(image)));
    };
};

vector<vector<Pixel>> p6(vector<vector<Pixel>> image, int firstInt, int secondInt) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage((secondInt * height), vector<Pixel>(firstInt * width));

   for (int row = 0; row < secondInt * height; ++row) {
        for (int col = 0; col < firstInt * width; ++col) {
            int originalRow = row / secondInt;
            int originalCol = col / firstInt;
            Pixel pixel = image[originalRow][originalCol];
            newImage[row][col] = pixel;
        }
    }
   return newImage; 
}

vector<vector<Pixel>> p7(vector<vector<Pixel>> image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(height, vector<Pixel>(width));

   for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            int red_value = pixel.red;
            int green_value = pixel.green;
            int blue_value = pixel.blue;
            int gray_value = (red_value + green_value + blue_value)/3;
            if (gray_value >= 255/2) {
                newImage[row][col].red = 255;
                newImage[row][col].green = 255;
                newImage[row][col].blue = 255;
            } else {
                newImage[row][col].red = 0;
                newImage[row][col].green = 0;
                newImage[row][col].blue = 0;
            }
        }
   }
   return newImage; 
}

vector<vector<Pixel>> p8(vector<vector<Pixel>> image, double scaler) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(height, vector<Pixel>(width));

   for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            int red_value = pixel.red;
            int green_value = pixel.green;
            int blue_value = pixel.blue;
            newImage[row][col].red = int(255 - (255 - red_value)*scaler);
            newImage[row][col].green = int(255 - (255 - green_value)*scaler);
            newImage[row][col].blue = int(255 - (255 - blue_value)*scaler);
        }
   }
   return newImage; 
}

vector<vector<Pixel>> p9(vector<vector<Pixel>> image, double scaler) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(height, vector<Pixel>(width));

   for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            int red_value = pixel.red;
            int green_value = pixel.green;
            int blue_value = pixel.blue;
            newImage[row][col].red = red_value*scaler;
            newImage[row][col].green = green_value*scaler;
            newImage[row][col].blue = blue_value*scaler;
        }
   }
   return newImage; 
}

vector<vector<Pixel>> p10(vector<vector<Pixel>> image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<Pixel>> newImage(height, vector<Pixel>(width));

   for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Pixel pixel = image[row][col];
            int red_value = pixel.red;
            int green_value = pixel.green;
            int blue_value = pixel.blue;
            int sum_color = red_value + green_value + blue_value;
            int max_color = max({red_value, green_value, blue_value});

            if (sum_color >= 550) {
                newImage[row][col] = {255, 255, 255};
            } else if (sum_color <= 150) {
                newImage[row][col] = {0, 0, 0};
            } else if (max_color == red_value) {
                newImage[row][col] = {255, 0, 0};
            } else if (max_color == green_value) {
                newImage[row][col] = {0, 255, 0};
            } else {
                newImage[row][col] = {0, 0, 255};
            }
        }
   }
   return newImage; 
}
///////////////////////END OF PROCESSES////////////////
///////////////////////STEP ONE////////////////////////
tuple<int, string> fileLoad(int counter, const vector<string>& msgs) {
    string fileName;
    bool fileNameCleared = false;
    int localCounter = counter;
    
    while (!fileNameCleared) {
        localCounter++;
        if (localCounter != 1) {
            cout << msgs[10];
        };
        
        cin >> fileName;
        if (fileName == "Q" || fileName == "q") {
            cout << "Goodbye" << endl;
            exit(0);
        } else if (fileName.empty()) {
            cout << jeevesRes(localCounter, msgs[4]) << endl;
        } else if (fileName.find(".bmp") == string::npos) {
            cout << jeevesRes(localCounter, msgs[3]);
        } else if (fileName != "Q" || fileName != "q") {
            fileNameCleared = true;
        }
    };
    return make_tuple(localCounter, fileName);
}

///////////////END OF STEP ONE//////////////////
///////////////STEP TWO////////////////////
    
int descriptionHub(int localCounter, const vector<string>& msgs) {
    bool returnToMenu = false;
    int choice;
    vector<string> opts = options();
    int receivedCounter = localCounter + 1;
    cout << jeevesRes(receivedCounter, msgs[7]);
    vector<string> descriptors =  descriptions();
    
    while (!returnToMenu) {
        receivedCounter++;
        for (int i = 0; i < opts.size(); ++i) {
            cout << i + 1 << ". " << opts[i] << endl;
        }
        cout << "--OR--" << endl;
        cout << 0 << ". " << "Back to Options" << '\n' << endl;
        cout << msgs[8];
        cin >> choice;
        
        if (cin.fail()) {
            cout << jeevesRes(receivedCounter, "Invalid input. Please enter a number.") << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 0) {
            returnToMenu = true;
        } else if (choice < 1 || choice > opts.size()) {
            int size = opts.size();
            string messageForJeeves = msgs[5] + to_string(size);
            cout << jeevesRes(receivedCounter, messageForJeeves) << endl;
            
        } else {
            int choicePicked = choice - 1;
            string messageForJeeves = descriptors[choicePicked] + "\n\n" + msgs[7];
            cout << jeevesRes(receivedCounter, messageForJeeves);
        }
    };
    return receivedCounter;
}
    
tuple<int, int> moreOptions(int counterPassed, const vector<string>& msgs) {
    bool returnToEffects = false;
    int choice;
    int localCounter = counterPassed;
    
    while (!returnToEffects) {
        localCounter++;
        cout << jeevesRes(localCounter, msgs[6]) << endl;
        cout << 1 << ". " << "Effect Descriptions" << endl;
        cout << 2 << ". " << "Back to Effects" << endl;
        cout << 3 << ". " << "Change the file being used" << endl;
        cout << 4 << ". " << "Exit the Application\n" << endl;
        cout << msgs[8];
        cin >> choice;
        
        if (cin.fail()) {
            cout << jeevesRes(localCounter, "Invalid input. Please enter a number.") << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        if (choice < 1 || choice > 3) {
            string messageForJeeves = msgs[5] + '3';
            cout << jeevesRes(localCounter, messageForJeeves) << endl;
        } else if (choice == 2) {
            returnToEffects = true;
        } else if (choice == 1) {
            int passedCounter = descriptionHub(localCounter, msgs);
            localCounter = passedCounter - 1;
        } else if (choice == 4) {
            cout << "Goodbye" << endl;
            exit(-1);
        } else if (choice == 3) {
            returnToEffects = true;
        } else {
            string messageForJeeves = msgs[5] + '3';
            cout << jeevesRes(localCounter, messageForJeeves) << endl;
        }
    }
    return make_tuple(localCounter, choice);
}

tuple<int, int> effectChosen(int counter, const vector<string>& msgs, bool reuse, string fileName) {
    string chooseEffectMessage = msgs[1];
    int choice;
    bool validChoice = false;
    vector<string> opts = options();
    int localCounter = 0;
    int passedCounter = counter;
    
    while (!validChoice) {
        localCounter++;
        passedCounter++;
        if (localCounter == 1 && reuse == false) {
            int jeevesCounter = passedCounter - 1;
            cout << jeevesRes(jeevesCounter, msgs[1]);
        } else {
            cout << jeevesRes(passedCounter, msgs[14]);
        }
        
        cout << "File name being used: " << fileName << ".\n" << endl;
                         
        for (int i = 0; i < opts.size(); ++i) {
            cout << i + 1 << ". " << opts[i] << endl;
        }
        cout << "--OR--" << endl;
        cout << 0 << ". " << "More Options" << '\n' << endl;
        cout << msgs[8];
        cin >> choice;
        
        if (cin.fail()) {
            cout << jeevesRes(localCounter, "Invalid input. Please enter a number.") << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        if (choice == 0) {
            int counterPassed = passedCounter - 1;
            tuple<int, int> optionsResults = moreOptions(counterPassed, msgs);
            int counterRecieved = get<0>(optionsResults);
            int optionsChoice = get<1>(optionsResults);
            passedCounter = counterRecieved;
            if (optionsChoice == 3) {
                choice = 303199433;
                validChoice = true;
            }
        } else if (choice < 1 || choice > opts.size()) {
            
            string messageForJeeves = msgs[5] + to_string(opts.size());
            
            cout << jeevesRes(passedCounter, messageForJeeves) << endl;
            
        } else {
            validChoice = true;
        }
    };
    return make_tuple(passedCounter, choice);
}

///////////////END OF STEP TWO////////////////
////////////////////////STEP THREE//////////////////////////
vector<vector<Pixel>> process(const vector<vector<Pixel>>& image, int chosenEffect, int counter, double scaler, int firstInt, int secondInt) {
    vector<vector<Pixel>> modifiedImage;
    
    if (chosenEffect == 1) {
        modifiedImage = p1(image);
    } else if (chosenEffect == 2) {
        modifiedImage = p2(image, scaler);
    } else if (chosenEffect == 3) {
        modifiedImage = p3(image);
    } else if (chosenEffect == 4) {
        modifiedImage = p4(image);
    } else if (chosenEffect == 5) {
        modifiedImage = p5(image, firstInt);
    } else if (chosenEffect == 6) {
        modifiedImage = p6(image, firstInt, secondInt);
    } else if (chosenEffect == 7) {
        modifiedImage = p7(image);
    } else if (chosenEffect == 8) {
        modifiedImage = p8(image, scaler);
    } else if (chosenEffect == 9) {
        modifiedImage = p9(image, scaler);
    } else if (chosenEffect == 10) {
        modifiedImage = p10(image);
    };
    

    return modifiedImage;
}

tuple<int, int> reUse(int counter, vector<string> msgs, string newFileName) {
    bool chosen = false;
    int toReuse = -1;
    int localCounter = counter;
    int jeevesCounter = counter + 1;
    string messageToSend = msgs[13] + newFileName;
    cout << jeevesRes(jeevesCounter, messageToSend);
    
    while (!chosen) {
        localCounter++;

        cout << 1 << ". " << "Use same file" << endl;
        cout << 2 << ". " << "Use a different file" << endl;
        cout << "--OR--" << endl;
        cout << 0 << ". " << "Quit" << '\n' << endl;
        cout << msgs[8];
        cin >> toReuse;
        
        if (cin.fail()) {
            cout << jeevesRes(localCounter, "Invalid input. Please enter a number.") << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        if (toReuse == 1) {
            chosen = true;
        } else if (toReuse == 0) {
            cout << "Goodbye\n";
            exit(0);
        } else if (toReuse == 2) {
            chosen = true;
        } else {
            jeevesRes(localCounter, "Sorry, that's not a choice. Pick from one of the options please.");
        }
    }
    
    return make_tuple(toReuse, localCounter);
}

tuple<int, string> makeFileName(int counter, int chosenEffect, string fileName) {
    int localCounter = counter;
    string fileNameChosen;
    bool userFinished = false;
    cout << jeevesRes(localCounter, "Let's name your file!");
    
    while (!userFinished) {
        counter++;
        cout << "Type the name you want, or type d for the default name. Then press enter: " << endl;
        cin >> fileNameChosen;
        
        if (cin.fail()) {
            cout << jeevesRes(counter, "Invalid input. Please try again with the title of your file.") << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        if (fileNameChosen == "d" || fileNameChosen == "D") {
            fileNameChosen = "process" + to_string(chosenEffect) + ".bmp";
            userFinished = true;
        } else if (fileNameChosen == fileName) {
            cout << jeevesRes(localCounter, "Sorry, that's the name of your original file. Try something different.");
        } else {
            if (fileNameChosen.size() >= 4 && fileNameChosen.substr(fileNameChosen.size() - 4) == ".bmp") {
                userFinished = true;
            } else {
                fileNameChosen += ".bmp";
                userFinished = true;
            }
        }
    }
    
    return make_tuple(localCounter, fileNameChosen);
}
    
///////////////END OF STEP THREE/////////////
///////////////////MAIN//////////////////////////
int main() {
    vector<string> msgs = jeevesDialogue();
    vector<vector<Pixel>> readImage;
    
    int counter = 0;
    int chosenEffect;
    
    string fileName;
    string introMessage = msgs[0];
    bool reuse = false;
    bool userFinished = false;
    bool changeFile = false;
    
    //begin
    cout << introMessage;
    
    while (!userFinished) {
        //cycle variables;
        double scaler = 0.0;
        int firstInt = 0;
        int secondInt = 0;
        
        //step1
        if (reuse == false) {
            tuple<int, string> stepOneResult = fileLoad(counter, msgs);
            counter = get<0>(stepOneResult);
            fileName = get<1>(stepOneResult);
            cout << "Filename: " << fileName << endl;
        }
        changeFile = false;
        
        while (!changeFile) {
        
            //step2
            tuple<int, int> result = effectChosen(counter, msgs, reuse, fileName);
            counter = get<0>(result);
            chosenEffect = get<1>(result);                        
            if (chosenEffect == 303199433) {
                changeFile = true;
            } else {
                if (chosenEffect == 2) {
                    bool scalerAdded = false;
                    while (!scalerAdded) {
                        counter++;
                        cout << jeevesRes(counter, "This requires a scaling factor");
                        cout << "Enter a number: ";
                        cin >> scaler;
                        if (cin.fail()) {
                            cout << jeevesRes(counter, "Invalid input. Please enter a number.") << endl;
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            continue;
                        } else {
                            scalerAdded = true;
                        }
                    }
                } else if (chosenEffect == 5) {
                    bool scalerAdded = false;
                    while (!scalerAdded) {
                        counter++;
                        cout << jeevesRes(counter, "Give me a number of times to rotate this clockwise by 90 degress");
                        cout << "Enter how many times to rotate: ";
                        cin >> firstInt;
                        if (cin.fail()) {
                            cout << jeevesRes(counter, "Invalid input. Please enter a number.") << endl;
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            continue;
                        } else {
                            scalerAdded = true;
                        }
                    }
                } else if (chosenEffect == 6) {
                    bool scalerAdded = false;
                    while (!scalerAdded) {
                        counter++;
                        cout << jeevesRes(counter, "If you wouldn't mind, could you tell me how much you'd like to enlarge the image by the x and y axis?");
                        cout << "X-Axis enlargement: ";
                        cin >> firstInt;
                        if (cin.fail()) {
                            cout << jeevesRes(counter, "Invalid input. Please enter a number.") << endl;
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            continue;
                        }
                        cout << "Y-Axis enlgargement: ";
                        cin >> secondInt;
                        if (cin.fail()) {
                            cout << jeevesRes(counter, "Invalid input. Please enter a number.") << endl;
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            continue;
                        } else {
                            scalerAdded = true;
                        }
                    }
                } else if (chosenEffect == 8) {
                    bool scalerAdded = false;
                    while (!scalerAdded) {
                        counter++;
                        cout << jeevesRes(counter, "Enter a Scalor Factor to lighten the image by");
                        cout << "Enter a number to darken the image by: ";
                        cin >> scaler;
                        if (cin.fail()) {
                            cout << jeevesRes(counter, "Invalid input. Please enter a number.") << endl;
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            continue;
                        } else {
                            scalerAdded = true;
                        }
                    }
                } else if (chosenEffect == 9) {
                    bool scalerAdded = false;
                    while (!scalerAdded) {
                        counter++;
                        cout << jeevesRes(counter, "Enter a Scalor Factor to darken the image by");
                        cout << "Enter a number to darken the image by: ";
                        cin >> scaler;
                        if (cin.fail()) {
                            cout << jeevesRes(counter, "Invalid input. Please enter a number.") << endl;
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            continue;
                        } else {
                            scalerAdded = true;
                        }
                    }
                };
                tuple<int, string> fileNameResult = makeFileName(counter, chosenEffect, fileName);
                string newFileName = get<1>(fileNameResult);
                counter = get<0>(fileNameResult);

                //step3
                readImage = read_image(fileName);
                vector<vector<Pixel>> modifiedImage = process(readImage, chosenEffect, counter, scaler, firstInt, secondInt);

                //step4
                bool success = write_image(newFileName, modifiedImage);
                if (!success) {
                    cout << jeevesRes(counter, "Sorry, there seemed to be an error. Let's try that again.");
                } else {
                    tuple<int, int> reuseResult = reUse(counter, msgs, newFileName);
                    int toReuse = get<0>(reuseResult);
                    counter = get<1>(reuseResult);
                    if (toReuse == 2) {
                        changeFile = true;
                    } else if (toReuse == 1) {
                        changeFile = false;
                    }
                }
            }
        }
    }

    return 0;
}