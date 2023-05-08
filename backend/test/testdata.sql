-- Delete existing data
DELETE FROM MMark13_Score WHERE id <= 7;
DELETE FROM DeviceType WHERE id <= 6;

-- Test data for DeviceType table
INSERT INTO DeviceType (id, added, disabled, manufacturer, model, product_name, os_version, device_type,
  total_ram, num_cpu_cores, cpu_type, cpu_max_frequency, screen_width, screen_height, gl_vendor, gl_renderer)
  VALUES 
  (1, now(), false, 'Apple', 'iPhone 4', null, 'iOS 5.0', 'mobilephone', 512000, 1, null, 1000, 960, 640, 'PowerVR', 'PowerVR SGX535'),
  (2, now(), false, 'Apple', 'iPhone 5', null, 'iOS 5.0', 'mobilephone', 1024000, 2, null, 1200, 1136, 640, 'PowerVR', 'PowerVR SGX543'),
  (3, now(), false, 'Samsung', 'S3', null, 'android 4.1', 'mobilephone', 512000, 4, null, 1400, 1280, 720, 'Mali', 'Mali-400MP'),
  (4, now(), false, 'Apple', 'iPad 4', null, 'iOS 6.0', 'tablet', 1024000, 2, null, 1400, 2048, 1536, 'PowerVR', 'PowerVR SGX554MP4'),
  (5, now(), false, 'Samsung', 'Nexus 10', null, 'android 4.1', 'tablet', 2048000, 2, null, 1700, 2592, 1936, 'Mali', 'Mali-T604'),
  (6, now(), false, 'Apple', 'iPad Mini', null, 'iOS 6.0', 'minitablet', 512000, 2, null, 1000, 1024, 768, 'PowerVR', 'PowerVR SGX543MP2');


-- Test data for MMark13_Score table
INSERT INTO MMark13_Score (id, added, version, uuid, client_submit_id, device_type_id, user_name, total_score, loadtime_score, 
  fractal_score, fractal_loadtime, fractal_num_images, fillrate_score, fillrate_loadtime, chess_score, chess_loadtime, chess_fps, 
  mountains_score, mountains_loadtime, mountains_fps, 
  unlighted_fillrate, vertex_lighted_fillrate, pixel_lighted_fillrate, mapped_lighted_fillrate)
  VALUES
  (1, now(), '1.0', uuid(), uuid(), 1, '', 5000, 100, 1000, 1.0, 22, 1000, 1.0, 1000, 1.0, 45.0, 1000, 2.0, 18.0, 100, 60, 40, 20),
  (2, now(), '1.0', uuid(), uuid(), 1, '', 5500, 600, 1100, 1.0, 22, 1200, 1.0, 1200, 1.0, 41.0, 1200, 2.0, 20.0, 110, 65, 50, 24),
  (3, now(), '1.0', uuid(), uuid(), 2, '', 6500, 800, 2100, 0.7, 29, 2200, 1.0, 2200, 1.0, 42.0, 2200, 2.0, 20.0, 210, 165, 150, 74),
  (4, now(), '1.0', uuid(), uuid(), 3, 'Foo', 6200, 810, 2000, 0.7, 29, 2000, 1.0, 2100, 1.0, 43.0, 2100, 2.0, 20.0, 200, 85, 90, 54),
  (5, now(), '1.0', uuid(), uuid(), 4, '', 6800, 760, 1700, 0.8, 26, 2220, 1.2, 2215, 1.5, 46.0, 2276, 2.0, 20.0, 240, 175, 180, 84),
  (6, now(), '1.0', uuid(), uuid(), 5, '', 6900, 820, 2130, 0.7, 31, 2223, 1.0, 2212, 1.0, 45.0, 2262, 2.0, 20.0, 208, 123, 112, 90),
  (7, now(), '1.0', uuid(), uuid(), 6, '', 4800, 520, 1730, 017, 22, 1923, 1.2, 1912, 1.1, 39.0, 1962, 2.2, 21.0, 138, 93, 82, 25);

