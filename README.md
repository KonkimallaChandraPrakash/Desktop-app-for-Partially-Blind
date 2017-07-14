# Desktop-app-for-Partially-Blind

For an Image this app divides the it into many patches and applies OTSU Binarization to each patch. This is to maximize the 
contrast between written and background for easy readability in case of partially blind

# Requirments
    OPENCV C++
    CMAKE

# Installation
    cd < directory >
    cmake .
    make

# Running
    ./main
    
# Settings
    cam_input to 0,1,2... according to camera device
