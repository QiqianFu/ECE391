### P1 Solution

# Q1 
- ## Necessary register value settings and how the VGA acts upon them
    - Line Compare field in the VGA specifies position of the horizontal division between the two windows. 
        - **Maximum Scan Line Register**: Bit 9
        - **Overflow Register**: Bit 8 
        - **Line Compare Register**: Bit 7-0
    - Smooth Panning and Scrolling needs Start Address field to specify the display memory address of the upper left pixel or character of the screen
        - **Start Address High Register and Start Address Low Register**
    - Pixel Panning Mode, this field allows the upper half of the screen to pan independently of the lower screen. If this field is set to 1, then upon a successful line compare, the bottom portion of the screen is displayed as if the Pixel Shift Count and Byte Panning fields are set to 0.
        - **Attribute Mode Control Register**: Bit 5 set to 1 
        - **Horizontal Pixel Panning Register**: Bit 3-0 set to 0
        - **Preset Row Scan Register**: Bit 6-5 set to 0
- ## Constraints 
    - The bottom window's starting display memory address is fixed at 0. This means that (unless you are using split screen mode to duplicate memory on purpose) the bottom screen must be located first in memory and followed by the top
    - The second limitation is that either both windows are panned by the same amount, or only the top window pans, in which case, the bottom window's panning values are fixed at 0. 
    - Another limitation is that the Preset Row Scan field only applies to the top window -- the bottom window has an effective Preset Row Scan value of 0.

# Q2
- ## Necessary register Operations
    - **Spin_lock to make sure it is atomic update**
    - **DAC Address Write Mode register**: select a palette entry that you want to change
     
    - **DAC Data register**: do output operation three times to the DAC Data register, change the R value, G value, B value seperately
    - **Spin_unlock**
