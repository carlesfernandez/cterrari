READ_DATA=$(/home/pi/cterrari/sht31-dual-reader -raw)
echo $READ_DATA | awk '{print $1}' | mosquitto_pub -h localhost -t "/terrari/temperatura" -l
echo $READ_DATA | awk '{print $2}' | mosquitto_pub -h localhost -t "/terrari/humitat" -l
echo $READ_DATA | awk '{print $3}' | mosquitto_pub -h localhost -t "/terrari/temperatura2" -l
echo $READ_DATA | awk '{print $4}' | mosquitto_pub -h localhost -t "/terrari/humitat2" -l
