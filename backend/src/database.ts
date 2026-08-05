import sqlite3 from 'sqlite3';
import path from 'path';

const dbPath = path.resolve(__dirname, '../central_backend.db');
export const db = new sqlite3.Database(dbPath);

export function initDatabase(): Promise<void> {
  return new Promise((resolve, reject) => {
    db.serialize(() => {
      // 1. users
      db.run(`
        CREATE TABLE IF NOT EXISTS users (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          email TEXT UNIQUE NOT NULL,
          password_hash TEXT NOT NULL,
          role TEXT NOT NULL,
          name TEXT NOT NULL,
          created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
      `);

      // 2. residents
      db.run(`
        CREATE TABLE IF NOT EXISTS residents (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          name TEXT NOT NULL,
          apartment TEXT NOT NULL,
          target_floor INTEGER NOT NULL,
          face_enrolled INTEGER DEFAULT 0,
          created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
      `);

      // 3. bills
      db.run(`
        CREATE TABLE IF NOT EXISTS bills (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          apartment TEXT NOT NULL,
          period TEXT NOT NULL,
          amount REAL NOT NULL,
          due_date TEXT NOT NULL,
          status TEXT DEFAULT 'UNPAID'
        )
      `);

      // 4. payments
      db.run(`
        CREATE TABLE IF NOT EXISTS payments (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          bill_id INTEGER NOT NULL,
          paid_at DATETIME DEFAULT CURRENT_TIMESTAMP,
          method TEXT NOT NULL
        )
      `);

      // 5. notifications
      db.run(`
        CREATE TABLE IF NOT EXISTS notifications (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          title TEXT NOT NULL,
          content TEXT NOT NULL,
          scope TEXT DEFAULT 'ALL',
          target TEXT DEFAULT '',
          created_by TEXT DEFAULT 'Admin',
          created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
      `);

      // 6. maintenance_requests
      db.run(`
        CREATE TABLE IF NOT EXISTS maintenance_requests (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          resident_id INTEGER,
          resident_name TEXT NOT NULL,
          apartment TEXT NOT NULL,
          title TEXT NOT NULL,
          description TEXT NOT NULL,
          status TEXT DEFAULT 'PENDING',
          priority TEXT DEFAULT 'MEDIUM',
          assigned_to TEXT DEFAULT 'Kỹ thuật viên 1',
          created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
      `);

      // 7. access_logs
      db.run(`
        CREATE TABLE IF NOT EXISTS access_logs (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          resident_id INTEGER NOT NULL,
          device_id TEXT DEFAULT 'DEV_COM3',
          floor INTEGER NOT NULL,
          result TEXT DEFAULT 'SUCCESS',
          timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
      `);

      // 8. alerts
      db.run(`
        CREATE TABLE IF NOT EXISTS alerts (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          device_id TEXT DEFAULT 'DEV_COM3',
          reason TEXT NOT NULL,
          timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
          resolved INTEGER DEFAULT 0
        )
      `);

      // 9. devices
      db.run(`
        CREATE TABLE IF NOT EXISTS devices (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          name TEXT NOT NULL,
          ip_address TEXT NOT NULL,
          api_key TEXT NOT NULL,
          last_synced_at DATETIME DEFAULT CURRENT_TIMESTAMP,
          current_mode TEXT DEFAULT 'RECOGNIZING'
        )
      `);

      // Seed Initial Data if empty
      db.get('SELECT COUNT(*) as count FROM users', (err, row: any) => {
        if (!err && row.count === 0) {
          console.log('[BACKEND DB] Seeding initial data...');
          // Users
          db.run(`INSERT INTO users (email, password_hash, role, name) VALUES 
            ('admin@smartelevator.vn', 'admin123', 'ADMIN', 'Ban Quản Trị'),
            ('security@smartelevator.vn', 'sec123', 'SECURITY', 'Đội Bảo Vệ'),
            ('resident@smartelevator.vn', 'res123', 'RESIDENT', 'Nguyễn Văn Tuấn')
          `);

          // Residents
          db.run(`INSERT INTO residents (name, apartment, target_floor, face_enrolled) VALUES 
            ('Tuan A', 'P502', 5, 1),
            ('Tuan B', 'P804', 8, 1),
            ('Tuan C', 'P1005', 10, 0),
            ('Lê Thị Hoa', 'P301', 3, 0),
            ('Phạm Minh Đức', 'P702', 7, 0)
          `);

          // Bills
          db.run(`INSERT INTO bills (apartment, period, amount, due_date, status) VALUES 
            ('P502', '08/2026', 1250000, '2026-08-15', 'PAID'),
            ('P804', '08/2026', 1450000, '2026-08-15', 'UNPAID'),
            ('P1005', '08/2026', 1800000, '2026-08-15', 'UNPAID'),
            ('P301', '08/2026', 950000, '2026-08-15', 'UNPAID')
          `);

          // Notifications
          db.run(`INSERT INTO notifications (title, content, scope, created_by) VALUES 
            ('Lịch bảo trì thang máy Bố trí Tháng 8', 'Thang máy khu B sẽ tạm ngưng bảo trì từ 14h-16h ngày 10/08/2026.', 'ALL', 'Ban Quản Trị'),
            ('Thông báo đóng phí quản lý dịch vụ', 'Vui lòng thanh toán hóa đơn phí quản lý dịch vụ trước ngày 15 hàng tháng.', 'ALL', 'Kế Toán')
          `);

          // Maintenance Requests
          db.run(`INSERT INTO maintenance_requests (resident_id, resident_name, apartment, title, description, status, priority) VALUES 
            (1, 'Tuan A', 'P502', 'Bóng đèn hành lang bị nhấp nháy', 'Đèn LED trước cửa căn 502 bị chập chập', 'IN_PROGRESS', 'MEDIUM'),
            (2, 'Tuan B', 'P804', 'Khóa cửa thông minh tầng 8 bị kẹt', 'Mã số thỉnh thoảng nhận diện chậm', 'PENDING', 'HIGH')
          `);

          // Devices
          db.run(`INSERT INTO devices (name, ip_address, api_key, current_mode) VALUES 
            ('Camera Thang Máy Cabin #1', 'http://localhost:8080', 'DEV_SECRET_KEY_123', 'RECOGNIZING')
          `);
        }
        resolve();
      });
    });
  });
}
