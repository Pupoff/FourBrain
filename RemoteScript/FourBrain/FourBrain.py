import Live

from _Framework.ControlSurface import ControlSurface
from _Framework.EncoderElement import EncoderElement
from _Framework.ButtonElement import ButtonElement
from _Framework.InputControlElement import MIDI_CC_TYPE
from _Framework.DeviceComponent import DeviceComponent


# ====== CONFIG MIDI ======
NUM_KNOBS = 57
MIDI_CH   = 1          #
FIRST_CC  = 0         # 


#refer too arduino code
CC_PARAM_OPEN  = 57     # 57: activer/désactiver le device
CC_ON_OFF = 58    # 58: afficher/focus la vue Device/Detail


# ====== MAPPING PAR NOM (ordre stable) ======
# les NOMS EXACTS apparaissent dans Live / Default.cfg.
PARAM_ORDER = [
    "Active Amp Section",
    "Roses Amp Gain",
    "Roses Amp Bass",
    "Roses Amp Middle",
    "Roses Amp Treble",
    "Roses Amp Presence",
    "Roses Amp Blend",
    "Roses Amp Output",
    "Cherubs Amp Gain",
    "Cherubs Amp Bass",
    "Cherubs Amp Middle",
    "Cherubs Amp Treble",
    "Cherubs Amp Presence",
    "Cherubs Amp Channel",
    "Cherubs Amp Output",
    "Pink Amp Gain",
    "Pink Amp Bass",
    "Pink Amp Middle",
    "Pink Amp Treble",
    "Pink Amp Presence",
    "Pink Amp Master",
    "Pink Amp Output",
    "Delay Active",
    "Delay Mix",
    "Delay Feedback",
    "Delay Low Cut",
    "Delay High Cut",
    "Delay Note",
    "Delay Amount",
    "Delay Sync",
    "Delay Mode",
    "Delay Type",
    "Compressor Active",
    "Compressor Level",
    "Compressor Compression",
    "Compressor Attack",
    "Overdrive Active",
    "Overdrive Drive",
    "Overdrive Level",
    "Overdrive Tone",
    "Boost Active",
    "Boost Gain",
    "Boost Level",
    "Boost Bass",
    "Boost Treble",
    "Reverb Active",
    "Reverb Mix",
    "Reverb Decay",
    "Reverb Low Cut",
    "Reverb High Cut",
    "Reverb Shimmer",
    "Chorus Active",
    "Chorus Mix",
    "Amp Type",
    "Input Gain",
    "Output Gain",
    "Doubler Active"
]

# Si tu n'as pas encore 64 noms, on complètera avec None (les knobs restants seront libres)
if len(PARAM_ORDER) < NUM_KNOBS:
    PARAM_ORDER = PARAM_ORDER + [None] * (NUM_KNOBS - len(PARAM_ORDER))
else:
    PARAM_ORDER = PARAM_ORDER[:NUM_KNOBS]


# (Optionnel) alias si le plugin renomme certains params selon version/langue
ALIASES = {
    # "Delay Note": ["Delay Note", "Delay Time", "Time"],
}



class FourBrain(ControlSurface):

    def __init__(self, c_instance):
        super(FourBrain, self).__init__(c_instance)
        with self.component_guard():
            self._encoders = []
            self._param_listeners = {}
            self._device_on_param = None
            self._device_on_listener = None

            self.log_message("FourBrain loaded (64 knobs CC1..CC64 + ON/OFF CC65 + SHOW CC66)")

            self._setup_controls()

            # DeviceComponent = main bleue + Lock
            self._device_component = DeviceComponent()
            self._device_component.name = "FourBrain_Device"
            self.set_device_component(self._device_component)
            self._device_component.set_enabled(True)

            self.schedule_message(1, self._remap_now)

    # ---------- Live callbacks (blue hand / lock) ----------

    def set_appointed_device(self, device):
        super(FourBrain, self).set_appointed_device(device)
        self.schedule_message(1, self._remap_now)

    def lock_to_device(self, device):
        super(FourBrain, self).lock_to_device(device)
        self.schedule_message(1, self._remap_now)

    def unlock_from_device(self, device):
        super(FourBrain, self).unlock_from_device(device)
        self.schedule_message(1, self._remap_now)

    # ---------- Setup controls ----------

    def _setup_controls(self):
        # X encodeurs 
        for i in range(NUM_KNOBS):
            cc = FIRST_CC + i
            enc = EncoderElement(MIDI_CC_TYPE, MIDI_CH, cc, Live.MidiMap.MapMode.absolute)
            enc.name = "Enc_%d" % (i + 1)
            self._encoders.append(enc)

        # bouton ON/OFF device 
        self._btn_toggle_on = ButtonElement(True, MIDI_CC_TYPE, MIDI_CH, CC_ON_OFF)
        self._btn_toggle_on.name = "Toggle_Device_On"
        self._btn_toggle_on.add_value_listener(self._on_toggle_device_on)

        # bouton "show detail/device view"
        self._btn_show_detail = ButtonElement(True, MIDI_CC_TYPE, MIDI_CH, CC_PARAM_OPEN)
        self._btn_show_detail.name = "Show_Detail_Device"
        self._btn_show_detail.add_value_listener(self._on_show_detail)

    def disconnect(self):
        try:
            self._btn_toggle_on.remove_value_listener(self._on_toggle_device_on)
        except:
            pass
        try:
            self._btn_show_detail.remove_value_listener(self._on_show_detail)
        except:
            pass

        self._detach_device_on_listener()
        self._clear_param_listeners()
        self._release_all_encoders()

        super(FourBrain, self).disconnect()

    # ---------- UI actions ----------

    def _on_show_detail(self, value):
        if value <= 0:
            return
        try:
            app_view = self.application().view
            # affiche la zone "Detail" et la chaîne de devices
            app_view.show_view('Detail')
            app_view.show_view('Detail/DeviceChain')
            app_view.focus_view('Detail/DeviceChain')
        except Exception as e:
            self.log_message("Show detail failed: %s" % str(e))

    # ---------- Device getter ----------

    def _get_target_device(self):
        dc = self._device_component

        try:
            if hasattr(dc, "_device") and dc._device is not None:
                return dc._device
        except:
            pass

        try:
            dev = dc.device
            if callable(dev):
                dev = dev()
            return dev
        except:
            return None

    # ---------- Device On/Off ----------
    def _on_toggle_device_on(self, value):
        dev = self._get_target_device()
        if dev is None or callable(dev):
            return

        try:
            on_param = dev.parameters[0]  # Device On/Off

            if value >= 64:
                # force ON
                on_param.value = float(on_param.max)
            else:
                # force OFF
                on_param.value = float(on_param.min)

            # send feedback immediately
            self._send_device_on_feedback(on_param)

        except Exception as e:
            self.log_message("Set device on/off failed: %s" % str(e))


    def _attach_device_on_listener(self, dev):
        self._detach_device_on_listener()
        try:
            p = dev.parameters[0]
            self._device_on_param = p

            def _cb():
                self._send_device_on_feedback(p)

            self._device_on_listener = _cb
            p.add_value_listener(_cb)
            self._send_device_on_feedback(p)
        except:
            self._device_on_param = None
            self._device_on_listener = None

    def _detach_device_on_listener(self):
        if self._device_on_param is not None and self._device_on_listener is not None:
            try:
                self._device_on_param.remove_value_listener(self._device_on_listener)
            except:
                pass
        self._device_on_param = None
        self._device_on_listener = None

    def _send_device_on_feedback(self, on_param):
        # renvoie 127 si ON, 0 si OFF sur CC65
        try:
            is_on = 1 if float(on_param.value) >= 0.5 else 0
        except:
            is_on = 0
        status = 0xB0 | (MIDI_CH)
        self._send_midi((status, CC_ON_OFF, 127 if is_on else 0))

    # ---------- Mapping principal (par nom) ----------

    def _remap_now(self):
        dev = self._get_target_device()
        if dev is None or callable(dev):
            self._detach_device_on_listener()
            self._clear_param_listeners()
            self._release_all_encoders()
            return

        # listener pour le bouton ON/OFF
        self._attach_device_on_listener(dev)

        try:
            params = list(dev.parameters)
        except:
            self._clear_param_listeners()
            self._release_all_encoders()
            return

        # ignore Device On/Off dans la liste de recherche par nom
        if len(params) > 0:
            params = params[1:]

        by_name = {}
        for p in params:
            try:
                by_name.setdefault(p.name, []).append(p)
            except:
                pass

        self._clear_param_listeners()

        for i, enc in enumerate(self._encoders):
            wanted = PARAM_ORDER[i]
            if wanted is None:
                enc.release_parameter()
                continue

            name_candidates = [wanted] + ALIASES.get(wanted, [])

            target = None
            for nm in name_candidates:
                cands = by_name.get(nm, [])
                if cands:
                    target = cands[0]
                    break

            if target is None:
                enc.release_parameter()
                continue

            enc.connect_to(target)

            cb = self._make_param_callback(i, target)
            try:
                target.add_value_listener(cb)
                self._param_listeners[target] = cb
            except:
                pass

            self._send_param_feedback(i, target)

        try:
            self.log_message("Mapped '%s' to CC%d..CC%d (by name)" %
                             (dev.name, FIRST_CC, FIRST_CC + NUM_KNOBS - 1))
        except:
            pass

    def _release_all_encoders(self):
        for enc in self._encoders:
            try:
                enc.release_parameter()
            except:
                pass

    def _clear_param_listeners(self):
        for p, cb in list(self._param_listeners.items()):
            try:
                p.remove_value_listener(cb)
            except:
                pass
        self._param_listeners.clear()

    # ---------- Feedback ----------

    def _make_param_callback(self, knob_index, param):
        def _cb():
            self._send_param_feedback(knob_index, param)
        return _cb

    def _send_param_feedback(self, knob_index, param):
        cc = FIRST_CC + knob_index
        val = self._param_to_midi_0_127(param)
        status = 0xB0 | (MIDI_CH)
        self._send_midi((status, cc, val))

    def _param_to_midi_0_127(self, param):
        try:
            v = float(param.value)
            mn = float(param.min)
            mx = float(param.max)
            if mx <= mn:
                return 0
            norm = (v - mn) / (mx - mn)
            if norm < 0.0:
                norm = 0.0
            elif norm > 1.0:
                norm = 1.0
            return int(round(norm * 127.0))
        except:
            return 0


def create_instance(c_instance):
    return FourBrain(c_instance)