import express, { Request, Response, NextFunction } from 'express';
import cors from 'cors';
import axios from 'axios';
import { db, initDatabase } from './database';

const app = express();
const PORT = process.env.PORT || 3000;
const DEVICE_IP = process.env.DEVICE_IP || 'http://localhost:8080';
const DEVICE_API_KEY = 'DEV_SECRET_KEY_123';

app.use(cors());
app.use(express.json());

// ---------------------------------------------------------
// Middleware Xác thực API Key cho Thiết bị Biên
// ---------------------------------------------------------
function requireApiKey(req: Request, res: Response, next: NextFunction) {
  const apiKey = req.header('X-API-Key');
  if (apiKey !== DEVICE_API_KEY) {
    return res.status(401).json({ error: 'Unauthorized: Invalid API Key' });
  }
  next();
}

// ---------------------------------------------------------
// 1. AUTHENTICATION API
// ---------------------------------------------------------
app.post('/api/auth/login', (req: Request, res: Response) => {
  const { email, password } = req.body;
  db.get('SELECT * FROM users WHERE email = ? AND password_hash = ?', [email, password], (err, user: any) => {
    if (err || !user) {
      return res.status(401).json({ error: 'Sai email hoặc mật khẩu' });
    }
    res.json({
      token: `fake-jwt-token-${user.id}-${Date.now()}`,
      user: {
        id: user.id,
        email: user.email,
        name: user.name,
        role: user.role
      }
    });
  });
});

// ---------------------------------------------------------
// 2. RESIDENT MANAGEMENT APIs (CRUD + Proxy Enrollment)
// ---------------------------------------------------------
app.get('/api/residents', (req: Request, res: Response) => {
  db.all('SELECT * FROM residents ORDER BY id ASC', [], (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

app.post('/api/residents', (req: Request, res: Response) => {
  const { name, apartment, target_floor } = req.body;
  if (!name || !apartment || !target_floor) {
    return res.status(400).json({ error: 'Thiếu thông tin name, apartment hoặc target_floor' });
  }
  db.run(
    'INSERT INTO residents (name, apartment, target_floor, face_enrolled) VALUES (?, ?, ?, 0)',
    [name, apartment, target_floor],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      res.json({ id: this.lastID, name, apartment, target_floor, face_enrolled: 0 });
    }
  );
});

app.put('/api/residents/:id', (req: Request, res: Response) => {
  const { id } = req.params;
  const { name, apartment, target_floor, face_enrolled } = req.body;
  db.run(
    'UPDATE residents SET name = ?, apartment = ?, target_floor = ?, face_enrolled = ? WHERE id = ?',
    [name, apartment, target_floor, face_enrolled ? 1 : 0, id],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      res.json({ status: 'ok', updated: this.changes });
    }
  );
});

app.delete('/api/residents/:id', (req: Request, res: Response) => {
  const { id } = req.params;
  db.run('DELETE FROM residents WHERE id = ?', [id], function (err) {
    if (err) return res.status(500).json({ error: err.message });
    res.json({ status: 'ok', deleted: this.changes });
  });
});

// Proxy kích hoạt Đăng ký khuôn mặt từ xa tới C++ Edge Device
app.post('/api/residents/:id/start-enrollment', async (req: Request, res: Response) => {
  const { id } = req.params;
  const residentId = parseInt(id, 10);

  try {
    // Gọi HTTP POST tới thiết bị biên C++
    const deviceRes = await axios.post(`${DEVICE_IP}/device/start-enrollment`, { residentId }, { timeout: 3000 });
    return res.status(deviceRes.status).json(deviceRes.data);
  } catch (error: any) {
    if (error.response) {
      return res.status(error.response.status).json(error.response.data);
    }
    return res.status(503).json({
      error: 'Không thể kết nối tới Thiết bị biên C++ SmartElevator. Kiểm tra camera server!'
    });
  }
});

// Proxy kiểm tra trạng thái Enrollment từ C++ Edge Device
app.get('/api/residents/:id/enrollment-status', async (req: Request, res: Response) => {
  const { id } = req.params;
  const residentId = parseInt(id, 10);

  try {
    const deviceRes = await axios.get(`${DEVICE_IP}/device/status`, { timeout: 3000 });
    const data = deviceRes.data;

    // Nếu progress đạt 30 và mode đã quay lại RECOGNIZING -> Cập nhật face_enrolled = 1 trong DB
    if (data.mode === 'RECOGNIZING' && data.enrollProgress >= 30) {
      db.run('UPDATE residents SET face_enrolled = 1 WHERE id = ?', [residentId]);
    }

    return res.json(data);
  } catch (error: any) {
    return res.status(503).json({
      mode: 'RECOGNIZING',
      enrollProgress: 0,
      elevatorState: 'OFFLINE',
      currentFloor: 1,
      targetFloor: 1,
      error: 'C++ Device offline'
    });
  }
});

// ---------------------------------------------------------
// 3. BILLS & PAYMENTS APIs
// ---------------------------------------------------------
app.get('/api/bills', (req: Request, res: Response) => {
  db.all('SELECT * FROM bills ORDER BY id DESC', [], (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

app.post('/api/bills', (req: Request, res: Response) => {
  const { apartment, period, amount, due_date } = req.body;
  db.run(
    'INSERT INTO bills (apartment, period, amount, due_date, status) VALUES (?, ?, ?, ?, "UNPAID")',
    [apartment, period, amount, due_date],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      res.json({ id: this.lastID, apartment, period, amount, due_date, status: 'UNPAID' });
    }
  );
});

app.post('/api/bills/:id/pay', (req: Request, res: Response) => {
  const { id } = req.params;
  const { method = 'Chuyển khoản QR' } = req.body;

  db.run('UPDATE bills SET status = "PAID" WHERE id = ?', [id], function (err) {
    if (err) return res.status(500).json({ error: err.message });
    db.run('INSERT INTO payments (bill_id, method) VALUES (?, ?)', [id, method]);
    res.json({ status: 'ok', message: 'Thanh toán thành công' });
  });
});

// ---------------------------------------------------------
// 4. NOTIFICATIONS APIs
// ---------------------------------------------------------
app.get('/api/notifications', (req: Request, res: Response) => {
  db.all('SELECT * FROM notifications ORDER BY id DESC', [], (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

app.post('/api/notifications', (req: Request, res: Response) => {
  const { title, content, scope = 'ALL', created_by = 'Admin' } = req.body;
  db.run(
    'INSERT INTO notifications (title, content, scope, created_by) VALUES (?, ?, ?, ?)',
    [title, content, scope, created_by],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      res.json({ id: this.lastID, title, content, scope, created_by });
    }
  );
});

// ---------------------------------------------------------
// 5. MAINTENANCE REQUESTS APIs
// ---------------------------------------------------------
app.get('/api/maintenance-requests', (req: Request, res: Response) => {
  db.all('SELECT * FROM maintenance_requests ORDER BY id DESC', [], (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

app.post('/api/maintenance-requests', (req: Request, res: Response) => {
  const { resident_id, resident_name, apartment, title, description, priority = 'MEDIUM' } = req.body;
  db.run(
    'INSERT INTO maintenance_requests (resident_id, resident_name, apartment, title, description, priority) VALUES (?, ?, ?, ?, ?, ?)',
    [resident_id, resident_name, apartment, title, description, priority],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      res.json({ id: this.lastID, resident_name, title, status: 'PENDING' });
    }
  );
});

app.put('/api/maintenance-requests/:id', (req: Request, res: Response) => {
  const { id } = req.params;
  const { status, assigned_to } = req.body;
  db.run(
    'UPDATE maintenance_requests SET status = ?, assigned_to = ? WHERE id = ?',
    [status, assigned_to, id],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      res.json({ status: 'ok', updated: this.changes });
    }
  );
});

// ---------------------------------------------------------
// 6. ACCESS LOGS & ALERTS (For Dashboard & Security UI)
// ---------------------------------------------------------
app.get('/api/access-logs', (req: Request, res: Response) => {
  db.all(
    `SELECT access_logs.*, residents.name as resident_name, residents.apartment 
     FROM access_logs 
     LEFT JOIN residents ON access_logs.resident_id = residents.id 
     ORDER BY access_logs.id DESC LIMIT 50`,
    [],
    (err, rows) => {
      if (err) return res.status(500).json({ error: err.message });
      res.json(rows);
    }
  );
});

app.get('/api/alerts', (req: Request, res: Response) => {
  db.all('SELECT * FROM alerts ORDER BY id DESC LIMIT 50', [], (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

// ---------------------------------------------------------
// 7. DEVICE SYNC ENDPOINTS (Authenticated with X-API-Key)
// ---------------------------------------------------------
app.get('/api/device/residents', requireApiKey, (req: Request, res: Response) => {
  db.all('SELECT id, name, apartment, target_floor FROM residents', [], (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

app.post('/api/device/access-logs', requireApiKey, (req: Request, res: Response) => {
  const { residentId, floor, timestamp } = req.body;
  db.run(
    'INSERT INTO access_logs (resident_id, floor, timestamp) VALUES (?, ?, ?)',
    [residentId, floor, timestamp || new Date().toISOString()],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      console.log(`[BACKEND LOG] Nhận Access Log: Resident ID ${residentId} -> Tầng ${floor}`);
      res.json({ status: 'ok', id: this.lastID });
    }
  );
});

app.post('/api/device/alerts', requireApiKey, (req: Request, res: Response) => {
  const { reason, timestamp } = req.body;
  db.run(
    'INSERT INTO alerts (reason, timestamp) VALUES (?, ?)',
    [reason, timestamp || new Date().toISOString()],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });
      console.log(`[BACKEND LOG] Nhận Cảnh báo An ninh: ${reason}`);
      res.json({ status: 'ok', id: this.lastID });
    }
  );
});

// ---------------------------------------------------------
// SERVER INITIALIZATION
// ---------------------------------------------------------
initDatabase().then(() => {
  app.listen(PORT, () => {
    console.log(`=======================================================`);
    console.log(`[BACKEND SERVER] NestJS/Express Central Backend Running!`);
    console.log(` -> URL: http://localhost:${PORT}`);
    console.log(` -> Device Proxy IP: ${DEVICE_IP}`);
    console.log(`=======================================================`);
  });
});
