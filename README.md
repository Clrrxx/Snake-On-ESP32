# Snake-On-ESP32
Using Platform IO and an ESP32, I created a Basic Snake Game on it. 

# Learning Points:
1. Data Structures & Spatial Awareness
The "Shape" of a Collection: A std::deque (double-ended queue) is a perfect data structure for Snake because it allows fast modifications at both ends. Able to add a new coordinate to the front (push_front) to   move the head, and drop the last item (pop_back) to simulate the tail moving forward.

Vector Mechanics vs. Memory Order: When working with coordinate systems, the order of elements matters. If movement direction is positive (moving right), internal array or deque data must place the head coordinate geometrically ahead of the rest of the body. Spawning items backwards triggers an unintended collision check on frame one. This caused much headache as the snake would keep spawning and killing itself instantly, resulting in an instant loss. 

2. Hardware vs. Software Coordinate Systems
The Inverted Y-Axis: In almost all display libraries (like TFT_eSPI), the pixel origin (0,0) sits at the top-left corner of the screen. Moving DOWN means adding to the Y-value (+1). Moving UP means subtracting from the Y-value (-1).

Flipping Vectors vs. Core Intent: When setting up opposite-direction safety checks (preventing the snake from folding into itself), have to evaluate movement rules based on the display's coordinate system, not real-world intuition. This caused some issues as I had to reorient certain values to match my inputs. 

3. Microcontroller Analog Inputs (ADCs)
Bit Resolution Dictates Scales: Different chips interpret voltage scales differently based on their Analog-to-Digital Converter (ADC) resolution:
Standard Arduino boards use a 10-bit ADC (2^10), producing values from 0 to 1023 (center approx 512).The ESP32 defaults to a 12-bit ADC (2^12), scaling values from 0 to 4095 (center approx 2048).Hardware values must always match the exact specifications of the architecture we are compiling code for. Hardware Centers Are Never Guaranteed. 

4. Input Polling and Signal Noise
The Multi-Sampling Overwrite Problem: Microcontrollers execute their primary control loops thousands of times faster than a user can physically interact with mechanical controls. If sample a joystick continuously without locking down the variable, tiny mechanical bounces or raw electrical noise will overwrite intentional inputs before the game code ticks forward to process them, resulting in the snake ignoring previous commands or not responding at all. Debouncing Applies to Analog Too. Without a deadzone, analog noise near the resting position constantly fires spurious directional inputs that corrupt the input buffer before the player can act.

Buffering Architecture: To build responsive controls, implement a lock-and-clear input strategy:Capture the input.If a valid move is captured, ignore subsequent variations until the next update tick.Execute the move, then clear the input register back to a blank state.

5. Bounded Randomization Math
The Modulo Offset Rule: To map a generic random number generator (rand()) to a custom physical boundary, we can systematically calculate exact boundaries with a modular offset formula. This allows to smoothly isolate object generation zones, keeping elements safely inside restricted margins or away from outer edges.

Since I already created Snake before, this simply is translating the original code from Raylib over to usable ESP32 code. 

  <img width="506" height="360" alt="ezgif-38cac0db183adbeb" src="https://github.com/user-attachments/assets/8f1ad94b-06c3-4011-a7dd-e8807f4114bc" />
  <img width="506" height="360" alt="ezgif-3814a9cd5c354de3" src="https://github.com/user-attachments/assets/0bb19c99-becd-498e-bd1d-4226e481ef70" />



