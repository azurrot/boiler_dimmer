
Using this module:

![image](https://github.com/user-attachments/assets/c5b22328-3900-4ad5-afee-bb1a66a9d547)




Fixing RBDdimmmer library for ESP8266:

Had the same thing. First resolved it by downgrading the ESP8266 board version to 2.5.0 in the board manager. However, the actual solution is adding in the file src/esp8266/RBDmcuESP8266.cpp the attribute ICACHE_RAM_ATTR  in front of the function isr_ext(), in the same way as it is in the function onTimerISR() (so void ICACHE_RAM_ATTR isr_ext())


## 🔌 MQTT AC-Dimmer Integration in Home Assistant

Dieses Projekt steuert einen AC-Dimmer (z. B. für einen Boiler) über einen ESP8266 per MQTT. Der ESP sendet die aktuelle Dimmerleistung als einfachen Integer-Wert (0–100) an das Topic `esp12/dimmer/state` und empfängt neue Werte über `esp12/dimmer/set`. Home Assistant übernimmt die Steuerung **ohne zusätzliche Firmware-Anpassung**.

---

### ⚙️ MQTT-Sensor in `configuration.yaml` einrichten

```yaml
sensor:
  - platform: mqtt
    name: "Boiler Dimmer Raw"
    state_topic: "esp12/dimmer/state"
    unit_of_measurement: "%"
    value_template: "{{ value | int }}"
```

Dieser Sensor stellt den aktuellen Dimmerwert (`0–100 %`) als `sensor.boiler_dimmer_raw` in Home Assistant bereit.

---

### 🔁 Automatisierung: Boiler-Leistung erhöhen (bei Stromüberschuss)

```yaml
alias: Boiler Leistung Erhöhen
trigger:
  - platform: time_pattern
    seconds: "/2"
condition:
  - condition: numeric_state
    entity_id: sensor.shellyem3_boiler_power
    below: -10
  - condition: numeric_state
    entity_id: sensor.boiler_dimmer_raw
    below: 100
action:
  - service: mqtt.publish
    data:
      topic: "esp12/dimmer/set"
      payload_template: >
        {{ [ (states('sensor.boiler_dimmer_raw') | int + 5), 100 ] | min }}
mode: single
```

---

### 🔁 Automatisierung: Boiler-Leistung reduzieren (bei hoher Last)

```yaml
alias: Boiler Leistung Reduzieren
trigger:
  - platform: time_pattern
    seconds: "/2"
condition:
  - condition: numeric_state
    entity_id: sensor.shellyem3_boiler_power
    above: 20
  - condition: numeric_state
    entity_id: sensor.shellyplus1pm_boiler_relais_power
    above: 5
  - condition: numeric_state
    entity_id: sensor.boiler_dimmer_raw
    above: 0
action:
  - service: mqtt.publish
    data:
      topic: "esp12/dimmer/set"
      payload_template: >
        {{ [ (states('sensor.boiler_dimmer_raw') | int - 5), 0 ] | max }}
mode: single
```

---

### ✅ Vorteile

- 🛠 Keine Firmware-Anpassung notwendig
- 📡 MQTT-Anbindung mit minimalem Setup
- 🧠 Flexible Automatisierung basierend auf Stromflüssen
- 💡 Direkte manuelle oder automatische Steuerung möglich

---

**Hinweis:** Stelle sicher, dass dein ESP die aktuellen Werte regelmäßig auf `esp12/dimmer/state` sendet, und dass das Topic `esp12/dimmer/set` korrekt verarbeitet wird.
