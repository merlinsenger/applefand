# applefand
Linux daemon for controlling the Apple iMac's display back light

Run manually i.e.

```
$ ./applefand
```

or install by copying to i.e. /usr/local/bin and create a systemd configuration, i.e.

```
merlin@imac:~/git$ cat /etc/systemd/system/applefand.service
[Unit]
Description=Apple display backlight brightness daemon

[Service]
Type=simple
ExecStart=/usr/local/bin/applefand

[Install]
WantedBy=multi-user.target
```

```
$ systemctl start applefand.service
$ systemctl enable applefand.service
```

Check the status using

```
$ systemctl status applefand.service

applefand.service - Apple fan control daemon
     Loaded: loaded (/etc/systemd/system/applefand.service; enabled; vendor preset: enabled)
     Active: active (running) since Mon 2021-09-13 11:32:59 CEST; 34min ago
   Main PID: 1080 (applefand)
      Tasks: 1 (limit: 19097)
     Memory: 608.0K
     CGroup: /system.slice/applefand.service
             └─1080 /usr/local/bin/applefand

Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 38°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 38°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 37°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 38°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 38°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 38°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 37°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 38°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 37°C, setting speed to 940rpm
Sep 13 12:06:19 imac applefand[1080]: applefand: CPU temperature is 36°C, setting speed to 940rpm


Sep 13 11:32:59 imac systemd[1]: Started Apple display backlight brightness daemon.
```

It has been some time already since this does not work anymore. Some process or driver is resetting the speed to default with seconds and faster. I have not figured out what is going on so far.
