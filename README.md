## Microservice to convert an ASCII-dump of CAN frames into PEAK envelopes

cat mydump.log| docker run -i --rm ghcr.io/chrberger/ascii2peak:latest [--verbose] [--id=1] > converted.rec
