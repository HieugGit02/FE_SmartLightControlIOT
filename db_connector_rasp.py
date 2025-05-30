import mysql.connector
from mysql.connector import Error

def get_db_connection():
    conn = None
    try:
        # conn = mysql.connector.connect(
        #     host="192.168.3.109",
        #     user="admin",
        #     password="admin",
        #     database="lamp_db",
        #     port=3306 
        # )
        conn = mysql.connector.connect(
            host="localhost",
            user="root",
            password="admin",
            database="lamp_db",
            port=3306 
        )
        print("✅ Connection to MariaDB (MySQL) successful!")
    except Error as e:
        print(f"❌ Error connecting to MariaDB: {e}")
        
    return conn

# import sqlite3

# def get_db_connection():
#     conn = sqlite3.connect('light.db')  # file SQLite bạn đang dùng
#     return conn
