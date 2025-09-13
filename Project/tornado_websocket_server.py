import tornado.ioloop
import tornado.web
import tornado.websocket
import serial
import threading
import MySQLdb
import MySQLdb.cursors
from datetime import datetime, date, time

DB_HOST = 'localhost'
DB_USER = 'rpi19'
DB_PASS = 'password'
DB_NAME = 'kv'

# List to keep track of connected clients
connected_clients = set()

# Establish a database connection
db_connection = MySQLdb.connect(host=DB_HOST, user=DB_USER, passwd=DB_PASS, db=DB_NAME)
db_cursor = db_connection.cursor(MySQLdb.cursors.DictCursor)



class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        # Register client
        connected_clients.add(self)
        print("New client connected")
        # Send the latest data from the database to the new client

    def on_message(self, message):
        # Handle incoming messages from clients if needed
        print(f"Received message from client: {message}")
        serial_port.write((message + '\n').encode('utf-8'))
        WebSocketHandler.insert_message_into_db(message)

    def on_close(self):
        # Unregister client
        connected_clients.remove(self)
        print("Client disconnected")

    @classmethod
    def send_message_to_clients(cls, message):
        for client in connected_clients:
            try:
                client.write_message(message)
                #print("Message sent")
            except Exception as e:
                print(f"Message not sent: {e}")

        # Insert message into the database
        cls.insert_message_into_db(message)

    @classmethod
    def insert_message_into_db(cls, message):
        stanje = None
        motion = None
        led = None
        rfid = None
        locked = None
        # Extract relevant information from the
        if message=="Motion detected!":
            stanje = "DOOR OPEN"
            led = "ON"
            rfid = "NOT DETECTED"
            locked = "UNLOCKED"
            motion = "DETECTED"
        elif message=="DOOR CLOSED":
            stanje = "DOOR CLOSED"
            led = "OFF"
            rfid = "NOT DETECTED"
            locked = "UNLOCKED"
            motion = "NOT DETECTED"
        elif message=="Authorized access":
            stanje = "DOOR OPEN"
            led = "ON"
            rfid = "DETECTED"
            locked = "UNLOCKED"
            motion = "NOT DETECTED"
        elif message=="DOOR LOCKED":
            stanje = "DOOR CLOSED"
            led = "OFF"
            rfid = "NOT DETECTED"
            locked = "LOCKED"
            motion = "NOT DETECTED"
        elif message=="REMOTE DOOR OPEN":
            stanje = "DOOR OPEN"
            led = "ON"
            rfid = "NOT DETECTED"
            locked = "UNLOCKED"
            motion = "NOT DETECTED"
        elif message=="REMOTE DOOR CLOSED":
            stanje = "DOOR CLOSED"
            led = "OFF"
            rfid = "NOT DETECTED"
            locked = "UNLOCKED"
            motion = "NOT DETECTED"
        elif message=="DOOR UNLOCKED":
            stanje = "DOOR CLOSED"
            led = "OFF"
            rfid = "NOT DETECTED"
            locked = "UNLOCKED"
            motion = "NOT DETECTED"


        if stanje or rfid or motion or led or locked:
            # Get the current date and time
            now = datetime.now()
            datum = now.date()
            vrijeme = now.time()

            # Insert the data into the sensorData table
            if locked == "UNLOCKED":
                query = """
                INSERT INTO sensorData (datum, vrijeme, stanje, rfid, motion, led, locked)
                VALUES (%s, %s, %s, %s, %s, %s, %s)
                """
                values = (datum, vrijeme, stanje, rfid, motion, led, locked)
                db_cursor.execute(query, values)
                db_connection.commit()
                print(f"Inserted message into database: {values}")
            elif locked == "LOCKED":
                query = """
                INSERT INTO sensorData (datum, vrijeme, stanje, rfid, motion, led, locked)
                VALUES (%s, %s, %s, %s, %s, %s, %s)
                """
                values = (datum, vrijeme, stanje, "NOT DETECTED", "NOT DETECTED", "OFF", locked)
                db_cursor.execute(query, values)
                db_connection.commit()
                print(f"Inserted message into database: {values}")


class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("index.html")  # Ensure this file is in the same directory as your script

def make_app():
    return tornado.web.Application([
        (r"/", MainHandler),
        (r"/ws", WebSocketHandler),
    ])

def read_from_serial(serial_port, ioloop):
    while True:
        if serial_port.in_waiting > 0:
            data = serial_port.readline().decode('utf-8').strip()
            print(f"Received data from serial port: {data}")
            ioloop.add_callback(WebSocketHandler.send_message_to_clients, data)

if __name__ == "__main__":
    app = make_app()
    app.listen(8888)
    print("WebSocket server is listening on ws://localhost:8888/ws")

    # Get the current IOLoop instance
    ioloop = tornado.ioloop.IOLoop.current()

    # Set up the serial port (adjust the port and baud rate as needed)
    serial_port = serial.Serial('/dev/ttyACM0', 9600)  # Example: '/dev/ttyUSB0' for Linux

    # Start a thread to read from the serial port
    serial_thread = threading.Thread(target=read_from_serial, args=(serial_port, ioloop))
    serial_thread.daemon = True
    serial_thread.start()

    ioloop.start()
