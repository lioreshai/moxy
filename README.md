

## Helpful Commands
### Tail all logs
```bash
journalctl --follow _SYSTEMD_UNIT=moxy.relays.service + _SYSTEMD_UNIT=moxy.sensors.service + _SYSTEMD_UNIT=moxy.panel.service
```

### Tail individual log
```bash
journalctl -u moxy.{serviceName}.service -f
```

### Restart all services
```
systemctl restart moxy.panel.service
systemctl restart moxy.sensors.service
systemctl restart moxy.relays.service
```