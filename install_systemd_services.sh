cp service_definitions/moxy.panel.service /etc/systemd/system
cp service_definitions/moxy.relays.service /etc/systemd/system
cp service_definitions/moxy.sensors.service /etc/systemd/system
systemctl daemon-reload
systemctl start moxy.panel
systemctl start moxy.relays
systemctl start moxy.sensors
echo "Installed. Tailing Logs:"
journalctl --follow _SYSTEMD_UNIT=moxy.relays.service + _SYSTEMD_UNIT=moxy.sensors.service + _SYSTEMD_UNIT=moxy.panel.service