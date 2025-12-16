import socket
import struct
import time

# Configuration
UDP_IP = "127.0.0.1" # Localhost
UDP_PORT = 1234

# Create UDP Socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Sending orders to {UDP_IP}:{UDP_PORT}...")

# Function to create a binary ITCH message
def create_packet(order_id, side, price, shares, stock="AAPL"):
    # Format: 
    # c (char type 'A')
    # H (uint16 locate)
    # H (uint16 tracking)
    # Q (uint64 timestamp)
    # Q (uint64 orderId)
    # c (char side)
    # I (uint32 shares)
    # 8s (char stock[8])
    # I (uint32 price)
    
    # Pack data into binary (Big Endian '>')
    # Price 100.00 -> 1000000
    price_int = int(price * 10000)
    
    packet = struct.pack('>cHHQQcI8sI', 
                         b'A',          # Type
                         1,             # Locate
                         1,             # Tracking
                         time.time_ns(),# Timestamp
                         order_id,      # Order ID
                         side.encode(), # Side ('B' or 'S')
                         shares,        # Shares
                         stock.ljust(8).encode(), # Stock (padded to 8 chars)
                         price_int      # Price
                        )
    return packet

# Send 10 orders
for i in range(1, 11):
    # Buy Order
    msg = create_packet(order_id=i, side='B', price=150.00, shares=10)
    sock.sendto(msg, (UDP_IP, UDP_PORT))
    print(f"Sent Buy Order {i}")
    time.sleep(0.5) # Wait a bit so we can see it happening

    # Sell Order (Matching)
    msg = create_packet(order_id=i+100, side='S', price=150.00, shares=10)
    sock.sendto(msg, (UDP_IP, UDP_PORT))
    print(f"Sent Sell Order {i+100}")
    time.sleep(0.5)

print("Done.")