apt-get install ros-melodic-rosserial-arduino
apt-get install ros-melodic-rosserial

## 启动 rosserial_server
roslaunch rosserial_server socket.launch

## getwsl ip
ip addr | grep eth0