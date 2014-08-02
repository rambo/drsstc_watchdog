# DRSSTC watchdog

Attiny based simple proxy to make sure my (to be done) MIDI modulator does not kill the IBGTs if it bugs out

## Working

  - Enfore pulse width (up to 254 uSec, but the max for my coil is 200 anyway...)
  - Enforce duty-cycle

