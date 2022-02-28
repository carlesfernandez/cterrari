# cterrari - Software pel terrari

## Material

- Raspberry Pi [Zero WH](https://www.raspberrypi.com/products/raspberry-pi-zero-w/).
- 2 sensors de temperatura i humitat [SHT31](https://es.aliexpress.com/item/1005003031689867.html?gatewayAdapt=glo2esp&spm=a2g0o.9042311.0.0.7f9763c0JrTIYZ).
  - Sensor 1 connectat a:
    - PWR a PIN 1
    - GND a PIN 6
    - SDA a PIN 3 (GPIO2)
    - SCL a PIN 5 (GPIO3)
  - Sensor 2 (opcional) connectat a:
    - PWR a PIN 17
    - GND a PIN 20
    - SDA a PIN 15 (GPIO22)
    - SCL a PIN 16 (GPIO23)

![GPIO mapping](https://cdn-media-1.freecodecamp.org/images/0*Zpa1YOQcMlvu-Sxs.png)

## Instal·lar Docker

A la Raspberry:

```
% curl -fsSL https://get.docker.com -o get-docker.sh
% sudo sh get-docker.sh
% sudo usermod -aG docker pi
% sudo mkdir /docker
% sudo chown pi /docker
```

## Instal·lar MQTT broker

A la Raspberry, fer espai per dades permanents:

```
% mkdir /docker/mosquitto
% mkdir /docker/mosquitto/config
% mkdir /docker/mosquitto/data
% mkdir /docker/mosquitto/log
```

Afegir aquest text a `/docker/mosquitto/config/mosquitto.conf`

```
%  nano /docker/mosquitto/config/mosquitto.conf
```

```
allow_anonymous true
listener 1883
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
```

Instal·lar mosquitto:

```
docker run -itd --name mosquitto --restart=always \
  -p 1883:1883 -p 9001:9001 \
  -v /docker/mosquitto/config:/mosquitto/config \
  -v /docker/mosquitto/data:/mosquitto/data \
  -v /docker/mosquitto/log:/mosquitto/log \
  eclipse-mosquitto
```

## Afegir port I2C per a un segon sensor (opcional)

A la Raspberry, instal·lar eines:

```
% sudo apt-get install i2c-tools
```

Per mapejar un altre port I2C a la Raspberry, afegir aquesta línea a
`/boot/config.txt`

```
% sudo nano /boot/config.txt
```

```
dtoverlay=i2c-gpio,bus=3,i2c_gpio_delay_us=1,i2c_gpio_sda=22,i2c_gpio_scl=23
```

Això mapeja un nou port I2C amb el SDA al GPIO22 (PIN 15) i el SCL al GPIO23
(PIN 16). El PIN 17 alimenta amb 3.3 V i el PIN 20 és terra.

Configurar permisos i reiniciar:

```
% sudo chown pi /dev/i2c-3
% sudo usermod -aG i2c pi
% sudo usermod -aG docker i2c
% sudo reboot
```

Aviam si va:

```
% i2cdetect -y 3
0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- 44 -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --  
```

## Instal·lar NodeRED

Fer espai per dades permanents:

```
% mkdir /docker/nodered
% mkdir /docker/nodered/data
```

Posar usuari `pi` com amo del port `/dev/i2c-1`

```
% sudo chown pi /dev/i2c-1
% sudo usermod -aG i2c pi
% sudo usermod -aG docker i2c
```

Instal·lar NodeRED:

```
% docker run -itd --name nodered --restart=always \
    -p 1880:1880 \
    -v /docker/nodered/data:/data \
    -e TZ=Europe/Madrid
    -e PGID=1000
    -e PUID=1000
    -u "1000:998"
    nodered/node-red:latest
```

## Instal·lar gestor Docker

Crear carpeta per dades permanents:

```
% mkdir /docker/portainer
% mkdir /docker/portainer/data
```

Instal·lar Portainer:

```
% docker run -itd --name portainer --restart=always \
    -p 8000:8000 -p 9000:9000 \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v /docker/portainer/data/:/data \
    portainer/portainer-ce:latest
```

## Compilar software lector del sensor SHT31

Instal·lar eines:

```
% sudo apt-get install build-essential git
```

Baixar el codi:

```
% git clone https://github.com/carlesfernandez/cterrari
```

Compilar:

```
% cd cterrari
% gcc ./sht31/sht31-dual.c -o sht31-dual-reader
```

Aviam si va:

```
% ./sht31-dual-reader
```


## Crontab

Editar crontab:
```
% crontab -e
```

Per a llegir el(s) sensor(s) cada minut, afegir:

```
* * * * * /scripts_raspberry/llegir_sensor_dual.sh
```

Per a llegir dades Raspberry cada minut, afegir:

```
* * * * * /scripts_raspberry/obtenir_dades_raspberry.sh
```
