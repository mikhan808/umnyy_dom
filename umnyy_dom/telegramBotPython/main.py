import socket
import telebot
from threading import Thread

current_temperature=0.0
current_humidity=0.0
current_spalnya=0
current_detskaya=0

def build_string_status(st):
    if st == 0:
        return "Выключен"
    if st == 1:
        return "Включен 1"
    if st == 2:
        return "Включен 2"
    if st == 3:
        return "Включен Все"
    return "Неизвестно"
# Задаем адрес сервера
SERVER_ADDRESS = ('192.168.0.120', 100)

# Настраиваем сокет
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(SERVER_ADDRESS)
server_socket.listen(10)
print('server is running, please, press ctrl+c to stop')
conns = list()

# Создаем экземпляр бота
bot = telebot.TeleBot('****')
# Функция, обрабатывающая команду /start
@bot.message_handler(commands=["start"])
def start(m, res=False):
    bot.send_message(m.chat.id, 'Я на связи. Напиши мне что-нибудь )')
# Получение сообщений от юзера
@bot.message_handler(content_types=["text"])
def handle_text(message):
    if message.text=="/read":
        global current_temperature
        global current_humidity
        msg="Свет в спальне "+build_string_status(current_spalnya)+"\n"
        msg+="Свет в детской "+build_string_status(current_detskaya)+"\n"
        msg+="Температура " + str(current_temperature) + "'C\n"
        msg += "Влажность " + str(current_humidity) + "%\n"
        bot.send_message(message.chat.id,msg)
    elif message.text=="/spalnya1on":
        serialSend([1,1,1])
    elif message.text == "/spalnya2on":
        serialSend([1,2,1])
    elif message.text == "/spalnya1off":
        serialSend([1, 1, 0])
    elif message.text == "/spalnya2off":
        serialSend([1, 2, 0])
    elif message.text == "/detskaya1on":
        serialSend([2, 1, 1])
    elif message.text == "/detskaya2on":
        serialSend([2, 2, 1])
    elif message.text == "/detskaya1off":
        serialSend([2, 1, 0])
    elif message.text == "/detskaya2off":
        serialSend([2, 2, 0])
    #bot.send_message(message.chat.id, 'Вы написали: ' + message.text)

def serialSend(data):
    txs = ""
    for val in data:
        txs += str(val)
        txs += ','
    txs = txs[:-1]
    txs += ';'
    for connection in conns:
        try:
            connection.send(txs.encode())
        except Exception :
           conns.remove(connection)
data_buf=""
def onRead():
    for connection in conns:
        rx=None
        try:
            rx = connection.recv(1024)
        except Exception:
            conns.remove(connection)
        if rx is None: return  # выходим если нечего читать
        global data_buf
        data_buf+=str(rx, 'utf-8').strip()
        data_bufs=data_buf.split(';')
        if len(data_bufs)<2:
            return
        data_buf=""
        i=0
        for s in data_bufs:
            if i!=0:
                if i != 1:
                    data_buf+=";"
                data_buf+=s
            i+=1
        rxs = data_bufs[0].strip()
        print(rxs)
        data = rxs.split(',')
        global current_spalnya
        global current_detskaya
        global current_temperature
        global current_humidity
        if data[0] == '1':
            current_spalnya=int(data[1])
        if data[0] == '2':
            current_detskaya = int(data[1])
        if data[0] == '3':
            current_temperature=float(data[1])
            print(current_temperature)
        if data[0] == '4':
            current_humidity=float(data[1])
            print(current_humidity)

        #connection.send(bytes('Hello', encoding='UTF-8'))

        #connection.close()


def startBot():
    bot.polling(none_stop=True, interval=0)
# Запускаем бота
def checkConnections():
    connection, address = server_socket.accept()
    print("new connection from {address}".format(address=address))
    conns.append(connection)

th = Thread(target=startBot)
th.start()
check_th=Thread(target=checkConnections)
check_th.start()

while True:
    onRead()






