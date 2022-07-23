all: build flash

build: build-ui build-firmware

build-ui:
	npm install --prefix ui
	npm run build --prefix ui
	pio run --target buildfs --environment heltec_wifi_lora_32_V2

build-firmware:
	pio run --environment heltec_wifi_lora_32_V2

flash: flash-ui flash-firmware

flash-ui:
	pio run --target uploadfs --environment heltec_wifi_lora_32_V2

flash-firmware:
	pio run --target upload --environment heltec_wifi_lora_32_V2