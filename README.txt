-------------------------------------------------------------------------------
COMP 4300 - Assignment 1
-------------------------------------------------------------------------------
How I implemented it:
-------------------------------------------------------------------------------

I built the program around a small ECS-style setup instead of keeping one big
shape vector in main. Each shape gets an entity id, and then its data is split
into components:

- TransformComponent stores the current position
- VelocityComponent stores how fast the shape moves
- RenderComponent stores the SFML shape and its material/color data

The Registry owns the component maps. Systems ask the Registry for the pieces
they need:

- MovementSystem updates position using velocity * dt
- WallCollisionSystem checks the shape against the window borders and flips the
  velocity when it hits a wall
- RenderSystem copies the TransformComponent position onto the SFML drawable,
  applies the material, and draws it

The ConfigLoader reads config.txt line by line. Window lines set the window
size, Font lines store font information, and shape lines create the correct
SFML shape, then add transform, velocity, and render components to the Registry.
This means the render loop does not care where the shapes came from. It just
runs the systems every frame.

The main loop is intentionally simple:

1. Read config.txt and load all entities into the Registry
2. Create the SFML window using the config size
3. Each frame, calculate dt
4. Handle close events
5. Move shapes
6. Bounce shapes off the walls
7. Clear, draw, and display

I used shared_ptr for the drawable because circles and rectangles are different
SFML types, but both can be treated as sf::Drawable/sf::Shape when drawing and
setting material values.

-------------------------------------------------------------------------------
Assignment Notes:
-------------------------------------------------------------------------------

Note: Please use the example code given for Lab 1 as the basis for this 
assignment. You will be required to submit main.cpp and NO OTHER FILES for this 
assignment. Submission will take place on D2L's Assignment 1 Dropbox. Only ONE
student from each group of two should submit the assignment.

Working solutions are included with the assignment so that you can see what a 
correct solution looks like. These executables are located in the 'bin' 
directory, and are provided for the lab Linux / Windows.

VERY IMPORTANT:
- Only submit main.cpp and no other files via D2L
- Do not submit main.cpp inside a zip file
- Do not submit main.cpp inside a Word.doc
- Do not submit main.cpp with a project file for an IDE
- Only submit main.cpp and no other files

YOU MUST INCLUDE THE FOLLOWING INFO IN A COMMENT AT THE TOP OF main.cpp:
- BOTH students' full names, cs usernames, and student numbers
- If you did not get a specific feature to work (such as text centering)
  please explain what you tried to do to get it to work, and partial marks
  may be given for effort.

-------------------------------------------------------------------------------
Program Specification
-------------------------------------------------------------------------------

In this assignment you will be writing a C++/SFML program which will read
descriptions of shapes from a configuration file, and then drawing those shapes
to the screen. Each shape will have a name, position, speed, color, as well as 
properties unique to each shape type. For example, a Rectangle will have width 
and height, and a circle will have a Radius. For this assignment, the position
of a shape refers to its SFML default of the upper-left corner of the shape.

You are also required to make these shapes "bounce" off the sides of the window 
which contains them. This means that if the shape's left side hits the left side
of the window, its X speed reverses. If the top of the shape hits the top of the
window, its Y speed reverses. Similiarly, if it hits the right side or bottom of
the window. You can assume that all shapes will always be configured to start 
entirely within the window, and you don't have to worry about what happens if 
they start outside the window. Each shape's name should be drawn on the exact 
center of the shape in the given font size and color specified in the file.
(note: this may be the trickiest part of the assignment, you will not lose many 
marks if it's not the exact center)mn

You Are also required to implement an imgui user interface which is able to
- list all of the shapes and select any one of them to edit their properties
- toggle whether or not the selected shape will be drawn
- Chnage the scale of the shape from 0 to (up to) 4
- change x and y velocity of the shape from -8 up to 8
- change the color of the shape
- change the name of the shape

A sample configuration file is given to you in config.txt, however marking will
be done with an unseen configuration file in order to test more completely. Be
sure to add and remove your own shapes to this file to test if they all work
correctly. 

Each line of the configuration file specifies one of the config settings of the 
assignment program, with the first string in each line being the type of setting
that the rest of the line specified. Lines in the config file can be one of the 
following types, and lines can appear in any order in the file.

Window W H
- This line declares that the SFML Window must be constructed
  with width W and height H, each of which will be integers
  
Font F S R G B
- This lines defines the font which is to be used to draw text
  for this program. The format of the line is as follows:
  Font File        F        std::string (it will have no spaces)
  Font Size        S        int
  RGB Color        (R,G,B)  int, int, int
  
Rectangle N X Y SX SY R G B W H
- Defines a RectangleShape with:
  Shape Name       Name     std::string
  Initial Position (X,Y)    float, float
  Initial Speed    (SX,SY)  float, float
  RGB Color        (R,G,B)  int, int, int
  Size             (W,H)    float, float
  
Circle N X Y SX SY R G B R
- Defines a CircleShape with:
  Shape Name       N        std::string
  Initial Position (X,Y)    float, float
  Initial Speed    (SX,SY)  float, float
  RGB Color        (R,G,B)  int, int, int
  Radius           R        float
  
-------------------------------------------------------------------------------
Assignment Hints
-------------------------------------------------------------------------------

- sf::CircleShape and sf::RectangeShape are both derived from sf::Shape
  Please use a std::shared_ptr<sf::Shape> variable to store all shapes.
  
  For example, if you have a vector of sf::Shape base pointers:
  example: std::vector< std::shared_ptr<sf::Shape> > shapes;
  
  You can then add a specific shape to this vector, for example:
  
  shapes.push_back(std::make_shared<sf::CircleShape>(radius));
  
  In order to draw this shape, you need to dereference the shared_ptr:
  
  window.draw(*shapes[i]);
  
- A shape will 'touch' the side of the window if its bounding box makes
  contact with it. Each shape's bounding rectangle can be obtained via:
  
  shape.getLocalBounds(); // .top, .left, .width, .height
  - gives the LOCAL position of the (top, left) of the rectangle
  - LOCAL pos means it is relative to shape.getPosition(), not the window
  - as well as the (width, height) size of the rectangle                          
                            
- Similarly, a sf::Text element's bounding rectangle can also be
  retrieved via text.getLocalBounds(), which you will need to use to
  center the text properly within a shape.
  
- Please use C++ file reading (std::ifstream makes this easy) rather
  than C-style FILE reading. You will lose marks for using older C functions!
