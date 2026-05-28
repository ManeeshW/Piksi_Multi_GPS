# Piksi Multi RTK Configuration Guide

## Table of Contents
- [How RTK Works (Base + Rover)](#how-rtk-works)
- [Universal Rules](#universal-rules)
- [Base Station Configuration](#base-station-configuration)
- [Rover Configuration](#rover-configuration)
- [Environment Profiles](#environment-profiles)
  - [Open Field / Ocean](#open-field--ocean)
  - [Urban / Buildings](#urban--buildings)
  - [Moving Base (Ship / Vehicle)](#moving-base-ship--vehicle)
- [Debugging RTK Fix Issues](#debugging-rtk-fix-issues)
- [Parameter Reference](#parameter-reference)

---

## How RTK Works

The base station sits at a **known, fixed location** and continuously broadcasts raw GNSS observations (pseudoranges and carrier phases) to the rover. The rover receives these corrections and computes a centimeter-accurate **differential position** relative to the base.

For RTK to achieve a **Fixed** solution:
1. Base and rover must observe the **same satellites** simultaneously
2. Corrections must arrive at the rover **fresh** (within `correction_age_max`)
3. Enough common satellites must exist for **integer ambiguity resolution** (typically 5+)
4. Satellite geometry must be strong (low PDOP, ideally < 3)

---

## Universal Rules

These apply in all environments on **both** base and rover.

### Both devices must match on:
- `enable_glonass`, `enable_galileo`, `enable_beidou` — enable the same constellations on both
- `glonass_measurement_std_downweight_factor` — must be identical
- `elevation_mask` in `[solution]` — must be identical (rover can be equal or lower than base)
- `dgnss_filter` and `dgnss_solution_mode`

### Base-only rules:
- `[track] mode = base station`
- `[surveyed_position] broadcast = True` with accurate lat/lon/alt
- The surveyed position **must match the physical antenna location**; re-survey if the antenna moves

### Rover-only rules:
- `[track] mode = rover`
- `[surveyed_position] broadcast = False`
- `[surveyed_position] surveyed_lat/lon/alt = 0` (leave zeroed)

---

## Base Station Configuration

```ini
[track]
mode = base station
elevation_mask = 5          # Track satellites above 5 degrees

[solution]
elevation_mask = 5
dgnss_filter = Fixed
dgnss_solution_mode = Time Matched
dynamic_motion_model = Static   # Base is not moving
correction_age_max = 30
enable_glonass = True
enable_galileo = True
enable_beidou = True
glonass_measurement_std_downweight_factor = 2
soln_freq = 5
output_every_n_obs = 1
disable_raim = False

[surveyed_position]
broadcast = True
surveyed_lat = <your accurate latitude>
surveyed_lon = <your accurate longitude>
surveyed_alt = <your accurate altitude in meters>

[rtcm_out]
output_mode = MSM5          # MSM5 supports GPS+GLONASS+Galileo+BeiDou

[acquisition]
almanacs_enabled = True
glonass_acquisition_enabled = True
galileo_acquisition_enabled = True
bds2_acquisition_enabled = True
sbas_acquisition_enabled = True
```

### How to get an accurate surveyed position
The base position accuracy directly limits your RTK accuracy. Three options ranked by quality:

1. **Average over time (built-in)** — Let the Piksi Multi run in standalone mode for 10–30 minutes, then read the averaged position from Swift Console. Use this as `surveyed_lat/lon/alt`.
2. **Known benchmark** — Place antenna over a surveyed control point and use its published coordinates.
3. **NTRIP/PPP** — Use an online service (e.g., OPUS, CSRS-PPP) with a 1-hour static log to get cm-level absolute accuracy.

---

## Rover Configuration

```ini
[track]
mode = rover
elevation_mask = 5

[solution]
elevation_mask = 5
dgnss_filter = Fixed
dgnss_solution_mode = Low Latency   # More robust than Time Matched
dynamic_motion_model = High Dynamics
correction_age_max = 60
enable_glonass = True
enable_galileo = True
enable_beidou = True
glonass_measurement_std_downweight_factor = 2
soln_freq = 5
output_every_n_obs = 1
disable_raim = False

[surveyed_position]
broadcast = False
surveyed_lat = 0
surveyed_lon = 0
surveyed_alt = 0
```

---

## Environment Profiles

### Open Field / Ocean

Open sky with no obstructions. You have maximum satellite visibility and minimal multipath. This is the ideal RTK environment.

**Characteristics:**
- Many satellites visible at all elevations
- Low multipath (signals arrive directly, no reflections)
- Strong satellite geometry (low PDOP)
- Radio link between base and rover is typically unobstructed

**Strategy:** Use stricter filters to get the cleanest, most accurate fix.

#### Base — Open Field / Ocean
```ini
[track]
elevation_mask = 10         # Can afford to cut low-elevation satellites (more atmospheric error)

[solution]
elevation_mask = 10
dgnss_filter = Fixed
dgnss_solution_mode = Time Matched
dynamic_motion_model = Static
correction_age_max = 30
glonass_measurement_std_downweight_factor = 2
disable_raim = False        # RAIM is effective with many clean satellites
```

#### Rover — Open Field / Ocean
```ini
[track]
elevation_mask = 10

[solution]
elevation_mask = 10
dgnss_filter = Fixed
dgnss_solution_mode = Low Latency
dynamic_motion_model = High Dynamics
correction_age_max = 30     # Shorter is fine with stable link
glonass_measurement_std_downweight_factor = 2
disable_raim = False
```

**Why higher elevation mask works here:**
Satellites near the horizon travel through more atmosphere, introducing more noise. In open sky you have plenty of high-elevation satellites, so excluding the low ones improves quality without reducing count.

---

### Urban / Buildings

Buildings cause **multipath** (reflections that look like satellites), **signal blockage** (no direct line of sight), and frequent satellite geometry changes. This is the hardest RTK environment.

**Characteristics:**
- Satellites blocked by buildings — low count, poor geometry
- Heavy multipath from glass/metal facades
- PDOP spikes frequently (poor geometry)
- Signal strength fluctuates as rover moves through urban canyons
- Radio link may be blocked by structures

**Strategy:** Accept more satellites (even low-elevation ones), be more tolerant of corrections age, and relax the fix filter.

#### Base — Urban / Buildings
Place the base on the **highest available point** (rooftop, elevated platform) to maximize sky view and minimize multipath at the base itself.

```ini
[track]
elevation_mask = 5          # Track everything you can

[solution]
elevation_mask = 5
dgnss_filter = Fixed
dgnss_solution_mode = Time Matched
dynamic_motion_model = Static
correction_age_max = 30
glonass_measurement_std_downweight_factor = 2
disable_raim = False
```

#### Rover — Urban / Buildings
```ini
[track]
elevation_mask = 5          # Track low satellites — you need every one you can get

[solution]
elevation_mask = 5          # Include low-elevation satellites in the solution
dgnss_filter = Float        # Accept float when fixed isn't possible (buildings block too many)
dgnss_solution_mode = Low Latency   # Essential — buildings cause radio link gaps
dynamic_motion_model = High Dynamics
correction_age_max = 60     # Longer tolerance for link interruptions
glonass_measurement_std_downweight_factor = 2  # Do NOT set this higher — you need GLONASS
disable_raim = True         # CAUTION: only disable if multipath keeps kicking out sats
                            # Try False first; switch to True only if sat count stays too low
```

**Urban-specific tuning notes:**

| Problem | Symptom | Fix |
|---|---|---|
| Too few satellites | PDOP > 5, no fix | Lower `elevation_mask` to 0–5, enable all constellations |
| Multipath false fixes | Position jumps by meters | Lower `correction_age_max`, keep `disable_raim = False` |
| Fix drops in building shadow | Intermittent fix | `dgnss_filter = Float` to keep a float solution |
| Radio link blocked by buildings | Corrections stale | Increase `correction_age_max` to 60–90 |
| RAIM kicking out too many sats | Drops below fix threshold | Try `disable_raim = True` carefully |

**Why `dgnss_filter = Float` in urban:**
A Float solution (dm-level accuracy) is better than no solution when buildings prevent the integer ambiguity resolution needed for Fixed (cm-level). You can always post-process to improve, but a dropped fix gives you nothing.

---

### Moving Base (Ship / Vehicle)

This is fundamentally different from all other scenarios. When the base is mounted on a **moving platform** (ship, boat, vehicle), it no longer has a known fixed position. The standard static RTK model breaks down and you must reconfigure accordingly.

**Characteristics:**
- Base position changes continuously — hardcoded `surveyed_lat/lon/alt` becomes immediately wrong
- Both units are in motion or one is relative to the other
- Ship superstructure can temporarily block satellite signals and the correction radio link
- Rolling and pitching causes antenna position to change even when the ship holds station

**Two use cases — pick the right one:**

| Use Case | Description | Rover absolute accuracy |
|---|---|---|
| **Heading / Attitude** | Both Piksi Multis on the same ship. Compute baseline vector → ship heading & attitude | Baseline is cm-accurate. Absolute position = base standalone (~1–3 m) |
| **Moving Base RTK** | Base on ship, rover on a drone/boat/robot near the ship. Rover position is relative to the moving base | Relative baseline cm-accurate. Absolute position limited by base's standalone GNSS (~1–3 m) |

> **Important limitation:** When the base is moving, the rover's absolute position accuracy is bounded by the base's standalone GNSS accuracy (~1–3 m with SBAS). The RTK baseline between them is still centimeter-accurate. If you need cm absolute positions on the rover, you need a land-based fixed reference station or NTRIP — not a moving base.

---

#### What breaks in your current `base_config.ini` when mounted on a ship

Your current base config has two critical problems for a moving base:

```ini
# PROBLEM 1 — base broadcasts its hardcoded land position as if it's still there
[surveyed_position]
broadcast = True                     # ← must change to False
surveyed_lat = 39.0608194961         # ← hardcoded Virginia coordinates
surveyed_lon = -77.4455431042        # ← rover will compute wrong baselines
surveyed_alt = 53.4655875264

# ALREADY CORRECT — dynamic_motion_model is High Dynamics
[solution]
dynamic_motion_model = High Dynamics  # ← this is correct for a ship
```

When `broadcast = True` with fixed coordinates and the base is physically elsewhere, the rover calculates the RTK baseline from the wrong origin. The rover's computed absolute position will be wrong by the full distance the base moved from Virginia.

---

#### Base — Moving Ship

```ini
[track]
mode = base station
elevation_mask = 5              # Open ocean: low mask captures all available satellites

[solution]
elevation_mask = 5
dgnss_filter = Fixed
dgnss_solution_mode = Time Matched
dynamic_motion_model = High Dynamics  # CRITICAL: ship is always moving
correction_age_max = 30
enable_glonass = True
enable_galileo = True
enable_beidou = True
glonass_measurement_std_downweight_factor = 2
soln_freq = 5
output_every_n_obs = 1
disable_raim = False
send_heading = False            # Set True if using two units for ship heading

[surveyed_position]
broadcast = False               # CRITICAL: do NOT broadcast a fixed position
surveyed_lat = 0                # Leave zeroed — base computes its own real-time position
surveyed_lon = 0
surveyed_alt = 0

[acquisition]
almanacs_enabled = True         # Helps re-acquire satellites faster after blockage
glonass_acquisition_enabled = True
galileo_acquisition_enabled = True
bds2_acquisition_enabled = True
sbas_acquisition_enabled = True # Enable SBAS for better standalone position on base
```

---

#### Rover — Moving Base Scenario (rover near the ship)

```ini
[track]
mode = rover
elevation_mask = 5

[solution]
elevation_mask = 5
dgnss_filter = Float            # Ship motion causes frequent ambiguity resets; Float is more stable
dgnss_solution_mode = Low Latency   # Essential — ship movement adds link timing variation
dynamic_motion_model = High Dynamics
correction_age_max = 60         # Ship superstructure may block radio briefly
enable_glonass = True
enable_galileo = True
enable_beidou = True
glonass_measurement_std_downweight_factor = 2
soln_freq = 5
output_every_n_obs = 1
disable_raim = False

[surveyed_position]
broadcast = False
surveyed_lat = 0
surveyed_lon = 0
surveyed_alt = 0
```

---

#### Heading Mode (two Piksi Multis on the same ship)

Use this when both units are rigidly mounted on the same vessel and you want **ship heading and attitude** (yaw, pitch, roll baseline).

**Unit 1 — "Base" (reference antenna, usually bow)**
```ini
[track]
mode = base station
elevation_mask = 5

[solution]
dynamic_motion_model = High Dynamics
send_heading = True             # Enable heading output
dgnss_filter = Fixed
dgnss_solution_mode = Low Latency

[surveyed_position]
broadcast = False               # Moving — never broadcast fixed coords
```

**Unit 2 — "Rover" (second antenna, usually stern or mast)**
```ini
[track]
mode = rover
elevation_mask = 5

[solution]
dynamic_motion_model = High Dynamics
send_heading = True
dgnss_filter = Fixed
dgnss_solution_mode = Low Latency
correction_age_max = 30         # Short — antennas are on same ship, link is reliable
```

The baseline vector between the two antennas gives:
- **Heading** = azimuth of the baseline
- **Pitch** = elevation angle of the baseline
- Accuracy: < 0.1° for a 1 m antenna separation baseline

---

#### Ship-specific tuning notes

| Problem | Symptom | Fix |
|---|---|---|
| Wrong absolute position | Rover position off by hundreds of km | Set `surveyed_position.broadcast = False` on base |
| Fix drops when ship rolls | Intermittent fix | Lower `elevation_mask` to 0–3, use `dgnss_filter = Float` |
| Corrections go stale | `correction_age_max` exceeded frequently | Increase to 60–90 s; check radio antenna placement |
| Satellite blockage by superstructure | Sat count drops suddenly | Enable all constellations; lower elevation mask |
| Heading jumps | Heading output unstable | Increase antenna separation; ensure rigid mounting |
| No fix with `Time Matched` | Base moving causes epoch mismatch | Switch rover to `dgnss_solution_mode = Low Latency` |

---

## Comparison Table: All Environments

| Parameter | Open Field / Ocean | Urban / Buildings | Moving Base — Ship |
|---|---|---|---|
| `track.elevation_mask` | 10 | 5 | 5 |
| `solution.elevation_mask` | 10 | 5 | 5 |
| `dgnss_filter` (rover) | Fixed | Float | Float |
| `dgnss_filter` (base) | Fixed | Fixed | Fixed |
| `dgnss_solution_mode` (rover) | Low Latency | Low Latency | Low Latency |
| `dgnss_solution_mode` (base) | Time Matched | Time Matched | Time Matched |
| `correction_age_max` (rover) | 30 | 60 | 60 |
| `glonass_measurement_std_downweight_factor` | 2 | 2 | 2 |
| `disable_raim` | False | False (try True if stuck) | False |
| `dynamic_motion_model` (rover) | High Dynamics | High Dynamics | High Dynamics |
| `dynamic_motion_model` (base) | Static | Static | **High Dynamics** |
| `surveyed_position.broadcast` (base) | True | True | **False** |
| `almanacs_enabled` | True | True | **True** (faster re-acquire) |
| Rover absolute accuracy | cm (if base surveyed accurately) | cm–dm | ~1–3 m (limited by base standalone) |
| Baseline accuracy | cm | cm–dm | cm |

---

## Debugging RTK Fix Issues

### Step 1 — Confirm corrections are flowing

Temporarily set on the rover:
```ini
[solution]
dgnss_filter = Float
```

| Result | What it means |
|---|---|
| Float solution appears | Corrections are arriving; problem is ambiguity resolution |
| No solution at all | Corrections not reaching rover — check radio link / port config |

### Step 2 — Check correction age

In Swift Console, watch the **Correction Age** field. If it stays > `correction_age_max`, the link is too slow or lossy. Increase `correction_age_max` or fix the physical link first.

### Step 3 — Check satellite count and PDOP

Need at minimum **5 common satellites** between base and rover for RTK. PDOP should be < 3 for reliable fixed solutions. In Swift Console:
- If fewer than 5 sats: lower `elevation_mask`, enable more constellations
- If PDOP > 5: no config change will fix this — wait for better satellite geometry or relocate

### Step 4 — Check surveyed position accuracy

If you get a Fixed solution but positions look wrong (off by meters vs known point), the base `surveyed_lat/lon/alt` is inaccurate. Re-survey the base.

### Step 5 — GLONASS inter-frequency issues

If you have GPS+GLONASS mixed and the fix is unstable, try:
```ini
glonass_measurement_std_downweight_factor = 4  # Reduce GLONASS influence
```
Or disable GLONASS entirely (`enable_glonass = False`) and test if GPS-only is stable.

### Step 6 — Time Matched vs Low Latency

If using `Time Matched` and corrections are arriving but fix is intermittent, switch rover to:
```ini
dgnss_solution_mode = Low Latency
```
`Time Matched` drops corrections if the rover's and base's observation timestamps don't align within one epoch — any radio latency causes this.

---

## Parameter Reference

| Parameter | Section | Description |
|---|---|---|
| `elevation_mask` | `[track]` | Minimum satellite elevation (degrees) to track. Lower = more sats, more noise. |
| `elevation_mask` | `[solution]` | Minimum elevation for satellites used in the position solution. |
| `dgnss_filter` | `[solution]` | `Fixed` = RTK fixed only (cm). `Float` = accept float (dm) when fixed unavailable. |
| `dgnss_solution_mode` | `[solution]` | `Time Matched` = strict epoch sync. `Low Latency` = tolerates link latency. |
| `correction_age_max` | `[solution]` | Seconds before a correction is considered too old and discarded. |
| `glonass_measurement_std_downweight_factor` | `[solution]` | Weight multiplier penalty on GLONASS (1=full weight, 4=quarter weight). |
| `disable_raim` | `[solution]` | RAIM excludes bad satellites. `False` = RAIM active (recommended). `True` = accept all satellites. |
| `dynamic_motion_model` | `[solution]` | `Static` for base, `High Dynamics` for moving rover. |
| `broadcast` | `[surveyed_position]` | `True` on base to broadcast its known position to rover. |
| `output_every_n_obs` | `[solution]` | Output solution every N observations. Keep at 1 for fastest updates. |
| `soln_freq` | `[solution]` | Solution output frequency in Hz. Max 10 Hz on Piksi Multi. |
| `mode` | `[track]` | `base station` or `rover`. Must be set correctly on each device. |
