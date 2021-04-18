
import alis

script_path = "./../../data/ishar/atari/depack/MAIN.AO"
script = alis.AlisScript(alis.ISHAR2_CRELITE, script_path, 0x22690)
lines = script.disasm()