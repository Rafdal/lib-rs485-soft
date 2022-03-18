#ifndef ASCII_DEFS_H
#define ASCII_DEFS_H

typedef enum {
    ASCII_NUL,    // Null char
    ASCII_SOH,    // Start of Heading
    ASCII_STX,    // Start of Text
    ASCII_ETX,    // End of Text
    ASCII_EOT,    // End of Transmission
    ASCII_ENQ,    // Enquiry
    ASCII_ACK,    // Acknowledgment
    ASCII_BEL,    // Bell
    ASCII_BS,     // Back Space
    ASCII_HT,     // Horizontal Tab
    ASCII_LF,     // Line Feed
    ASCII_VT,     // Vertical Tab
    ASCII_FF,     // Form Feed
    ASCII_CR,     // Carriage Return
    ASCII_SO,     // Shift Out / X-On
    ASCII_SI,     // Shift In / X-Off
    ASCII_DLE,    // Data Line Escape
    ASCII_DC1,    // Device Control 1 (oft. XON)
    ASCII_DC2,    // Device Control 2
    ASCII_DC3,    // Device Control 3 (oft. XOFF)
    ASCII_DC4,    // Device Control 4
    ASCII_NAK,    // Negative Acknowledgement
    ASCII_SYN,    // Synchronous Idle
    ASCII_ETB,    // End of Transmit Block
    ASCII_CAN,    // Cancel
    ASCII_EM,     // End of Medium
    ASCII_SUB,    // Substitute
    ASCII_ESC,    // Escape
    ASCII_FS,     // File Separator
    ASCII_GS,     // Group Separator
    ASCII_RS,     // Record Separator
    ASCII_US,     // Unit Separator
} ascii_chars_t;

#endif