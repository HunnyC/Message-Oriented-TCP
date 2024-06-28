# Message-Oriented-TCP
This project implements a message-oriented TCP-like protocol (MTP) over UDP, using socket programming in C. The protocol ensures reliable message delivery, similar to TCP, but operates over the connectionless UDP protocol. This project aims to combine the speed and simplicity of UDP with the reliability of TCP.

# Features:
Reliable Message Delivery: Ensures messages are delivered in order and without loss.
Acknowledgment Mechanism: Implements an acknowledgment system to confirm message receipt. 
Timeout and Retransmission: Handles packet loss by retransmitting messages after a timeout period. 
Sequencing: Maintains the order of messages using sequence numbers. 
Flow Control: Manages data flow to prevent overwhelming the receiver.

