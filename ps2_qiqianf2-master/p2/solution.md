### P2 Solution

## Q1


- **MTCP_BIOC_ON**: This message is sent when the user wants to turn on the button interrupt. This causes the Button interrupt-on-change to be enabled on the device and returns MTCP_ACK to the computer.

- **MTCP_LED_SET**: MTCP_LED_SET is sent when user want to turn on the LED. This causes the display to use the User-set LED display values. If LED is at USR mode these will be displayed on the LED display. MTCP_LED_SET returns  MTCP_ACK to the computer.

## Q2
- **MTCP_ACK**: This message is sent after every MTC command indicating that the MTC has successfully completed a command.

- **MTCP_BIOC_EVENT**: When the Button Interrupt-on-change mode is enabled, and a button is either pressed or released, this message is generated. It signifies that the user has touched the button and changed its status.

- **MTCP_RESET**: This message is generated when the device re-initializes itself, whether after a power-up, a RESET button press, or an MTCP_RESET_DEV command. It indicates that the device has re-initialized itself.




## Q3
Because the function is called from an interrupt context, if it waits or takes too much time, the processor
will not be able to handle other urgent event / will lose the data.