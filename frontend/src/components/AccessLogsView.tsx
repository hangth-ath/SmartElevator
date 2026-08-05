import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { ShieldAlert, Clock, AlertTriangle, CheckCircle2 } from 'lucide-react';
import { ElevatorIcon as Elevator } from './ElevatorIcon';
import { AccessLog, SecurityAlert } from '../types';

export const AccessLogsView: React.FC = () => {
  const [logs, setLogs] = useState<AccessLog[]>([]);
  const [alerts, setAlerts] = useState<SecurityAlert[]>([]);
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    setLoading(true);
    Promise.all([
      axios.get('/api/access-logs'),
      axios.get('/api/alerts')
    ]).then(([logsRes, alertsRes]) => {
      setLogs(logsRes.data);
      setAlerts(alertsRes.data);
      setLoading(false);
    }).catch(console.error);
  }, []);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '24px' }}>
      <div className="glass-panel" style={{ padding: '20px 24px', border: '1px solid rgba(244,63,94,0.3)' }}>
        <h3 style={{ fontSize: '1.1rem', fontWeight: 800, color: '#f43f5e', marginBottom: '14px', display: 'flex', alignItems: 'center', gap: '8px' }}>
          <ShieldAlert size={20} /> Cảnh Báo An Ninh Người Lạ ({alerts.length})
        </h3>

        <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
          {alerts.length === 0 ? (
            <div style={{ padding: '20px', color: '#10b981', textAlign: 'center' }}>Không phát hiện cảnh báo an ninh nào</div>
          ) : (
            alerts.map(a => (
              <div key={a.id} style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '12px 16px', background: 'rgba(244,63,94,0.08)', borderRadius: '10px', border: '1px solid rgba(244,63,94,0.2)' }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
                  <AlertTriangle size={20} color="#f43f5e" />
                  <div>
                    <div style={{ fontWeight: 700, color: '#f8fafc', fontSize: '0.9rem' }}>{a.reason}</div>
                    <div style={{ color: '#94a3b8', fontSize: '0.75rem' }}>
                      Thiết bị: {a.device_id} | Thời gian: {new Date(a.timestamp).toLocaleString('vi-VN')}
                    </div>
                  </div>
                </div>
                <span className="badge badge-danger">Cảnh Báo Nổi Bật</span>
              </div>
            ))
          )}
        </div>
      </div>

      <div className="glass-panel" style={{ padding: '20px 24px' }}>
        <h3 style={{ fontSize: '1.1rem', fontWeight: 800, color: '#06b6d4', marginBottom: '14px', display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Elevator size={20} /> Lịch Sử Nhận Diện AI & Quẹt Mặt Thang Máy
        </h3>

        <table style={{ width: '100%', borderCollapse: 'collapse', textAlign: 'left' }}>
          <thead>
            <tr style={{ background: 'rgba(255,255,255,0.04)', borderBottom: '1px solid rgba(255,255,255,0.08)', color: '#94a3b8', fontSize: '0.8rem' }}>
              <th style={{ padding: '12px 16px' }}>Thời Gian</th>
              <th style={{ padding: '12px 16px' }}>Cư Dân</th>
              <th style={{ padding: '12px 16px' }}>Căn Hộ</th>
              <th style={{ padding: '12px 16px' }}>Tầng Kích Hoạt</th>
              <th style={{ padding: '12px 16px' }}>Kết Quả</th>
            </tr>
          </thead>
          <tbody>
            {loading ? (
              <tr><td colSpan={5} style={{ textAlign: 'center', padding: '30px', color: '#64748b' }}>Đang tải nhật ký...</td></tr>
            ) : (
              logs.map(log => (
                <tr key={log.id} style={{ borderBottom: '1px solid rgba(255,255,255,0.05)' }}>
                  <td style={{ padding: '12px 16px', color: '#94a3b8', fontSize: '0.85rem' }}>
                    {new Date(log.timestamp).toLocaleString('vi-VN')}
                  </td>
                  <td style={{ padding: '12px 16px', fontWeight: 700, color: '#f8fafc' }}>
                    {log.resident_name || `Cư dân #${log.resident_id}`}
                  </td>
                  <td style={{ padding: '12px 16px', color: '#06b6d4', fontWeight: 700 }}>
                    {log.apartment || 'P502'}
                  </td>
                  <td style={{ padding: '12px 16px', fontWeight: 700, color: '#10b981' }}>
                    Tầng {log.floor}
                  </td>
                  <td style={{ padding: '12px 16px' }}>
                    <span className="badge badge-success"><CheckCircle2 size={12} /> Thành Công</span>
                  </td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};
