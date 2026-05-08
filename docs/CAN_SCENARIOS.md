# CAN scenario layer

Front32 now treats machine work as deterministic remote scenarios. A scenario is
started by Front32, executed locally by the target Back32 node, and then reported
back as `Done`, `Error`, `Timeout`, or `Canceled`.

Current node map:

| Node | CAN ID | Local hardware |
| --- | --- | --- |
| TABLE | `0x201` | motor TABLE, sensors TABLE_UP and TABLE_DOWN |
| GUILLOTINE | `0x202` | motor GUILLOTINE, sensor GUILLOTINE_HOME |
| PAPER | `0x203` | motor PAPER, sensors MARK and EDGE, encoder |
| THROW | `0x204` | motor THROW, start sensor |
| PAPER+THROW group | `0x220` | group start frame accepted by PAPER and THROW |

The scenario protocol lives in `../Can32/src/protocols/scenario/`.

Scenarios expected from Back32:

| Scenario | Node | Notes |
| --- | --- | --- |
| `TableUp` | TABLE | Move up and stop locally on TABLE_UP or timeout. |
| `TableDown` | TABLE | Move down and stop locally on TABLE_DOWN or timeout. |
| `GuillotineHome` | GUILLOTINE | Find home sensor. |
| `GuillotineCut` | GUILLOTINE | Full deterministic cut cycle. |
| `PaperMoveSteps` | PAPER | Move by encoder/motor steps. |
| `PaperZeroPosition` | PAPER | Reset local paper position/encoder origin. |
| `DetectPaper` | PAPER | Move until EDGE sensor, return encoder position. |
| `DetectMark` | PAPER | TODO: implement in CAN32/Back32 PAPER node, move until MARK sensor and return encoder position. |
| `ThrowRun` | THROW | Run throw motor using local start sensor rules. |
| `PaperThrowGroup` | PAPER and THROW | TODO: both nodes listen to group CAN ID and start from the same frame; status is read from individual node IDs. |

Important rule: guarded motion must not depend on Front32 polling sensors over
CAN. If a motor must stop on a sensor, that sensor belongs to the same remote
scenario node.
