import copy
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location('validator', ROOT / 'tools/selfcheck/validate_config.py')
v = importlib.util.module_from_spec(spec)
spec.loader.exec_module(v)


class ConfigValidation(unittest.TestCase):
    def setUp(self):
        self.schema = v.load_json(v.SCHEMA)
        self.config = v.load_json(ROOT / 'config/examples/capture.example.json')

    def test_design_example_requires_explicit_opt_in(self):
        self.assertTrue(v.validate(self.config, self.schema))
        self.assertEqual([], v.validate(self.config, self.schema, allow_incomplete=True))

    def test_production_config_and_rejections(self):
        self.config['camera']['device_mapping'] = [f'/dev/camera{i}' for i in range(5)]
        self.config['video'].update(bitrate_bps_per_camera=8000000, segment_seconds=60)
        self.config['imu']['sample_rate_hz'] = 1000
        self.config['storage']['root_path'] = '/data/tasks'
        self.config['sync'].update(clock_domain='host_monotonic', max_error_us=100)
        self.assertEqual([], v.validate(self.config, self.schema))
        # Values are fixtures, not hardware recommendations.
        for section, key, value in [('camera', 'fps', True), ('camera', 'device_mapping', ['/dev/a'] * 5), ('sync', 'max_error_us', -1), ('storage', 'root_path', 'relative'), ('video', 'unknown', 1)]:
            config = copy.deepcopy(self.config)
            config[section][key] = value
            with self.subTest(key=key):
                self.assertTrue(v.validate(config, self.schema))

    def test_duplicate_json_keys_rejected(self):
        with self.assertRaises(ValueError):
            v.unique_object([('fps', 30), ('fps', 60)])


if __name__ == '__main__':
    unittest.main()
