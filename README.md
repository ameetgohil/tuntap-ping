## Tun/Tap Ping example

```
> tuntap-ping
tun/tap ping example
> openvpn --mktun --dev tun77 --user waldner
Fri Mar 26 10:48:12 2010 TUN/TAP device tun77 opened
Fri Mar 26 10:48:12 2010 Persist state set to: ON
> ip link set tun77 up
> ip addr add 10.0.0.1/24 dev tun77
> ping 10.0.0.2
...

> on another console
$ ./tunclient
Read 84 bytes from device tun77
Read 84 bytes from device tun77
```

### Reference
https://backreference.org/2010/03/26/tuntap-interface-tutorial/
