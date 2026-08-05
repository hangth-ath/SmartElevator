import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Users, Building, DollarSign, ShieldAlert, CheckCircle2 } from 'lucide-react';
import { ElevatorIcon as Elevator } from './ElevatorIcon';
import { Resident, AccessLog, SecurityAlert } from '../types';

export const DashboardView: React.FC<{ onNavigate: (tab: string) => void }> = ({ onNavigate }) => {
  const [stats, setStats] = useState({
    totalResidents: 0,
    enrolledFaces: 0,
    totalApartments: 240,
    monthlyRevenue: 145000000,
    pendingMaintenance: 0
  });

  const [recentLogs, setRecentLogs] = useState<AccessLog[]>([]);
  const [alerts, setAlerts] = useState<SecurityAlert[]>([]);

  useEffect(() => {
    axios.get('/api/residents').then(res => {
      const residents: Resident[] = res.data;
      const enrolled = residents.filter(r => r.face_enrolled === 1).length;
      setStats(prev => ({
        ...prev,
        totalResidents: residents.length,
        enrolledFaces: enrolled
      }));
    }).catch(console.error);

    axios.get('/api/access-logs').then(res => {
      setRecentLogs(res.data.slice(0, 5));
    }).catch(console.error);

    axios.get('/api/alerts').then(res => {
      setAlerts(res.data.slice(0, 5));
    }).catch(console.error);
  }, []);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '24px' }}>
      <div className="glass-panel" style={{ padding: '24px', background: 'linear-gradient(135deg, rgba(6,182,212,0.15), rgba(59,130,246,0.15))', border: '1px solid rgba(6,182,212,0.3)', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <div>
          <h2 style={{ fontSize: '1.4rem', fontWeight: 800, color: '#f8fafc' }}>
            Chào mừng đến với Hệ Thống Quản Lý SmartElevator!
          </h2>
          <p style={{ color: '#94a3b8', fontSize: '0.9rem', marginTop: '4px' }}>
            Quản lý chung cư thông minh tích hợp nhận diện AI khuôn mặt & điều khiển thang máy từ xa.
          </p>
        </div>
        <button className="btn btn-primary" onClick={() => onNavigate('residents')}>
          <Users size={16} /> Thêm Cư Dân & Quét Khuôn Mặt
        </button>
      </div>

      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '20px' }}>
        <div className="glass-panel" style={{ padding: '20px', display: 'flex', alignItems: 'center', gap: '16px' }}>
          <div style={{ width: '48px', height: '48px', borderRadius: '12px', background: 'rgba(6, 182, 212, 0.15)', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
            <Users size={24} color="#06b6d4" />
          </div>
          <div>
            <div style={{ fontSize: '0.8rem', color: '#94a3b8', fontWeight: 600 }}>Cư Dân Đã Đăng Ký</div>
            <div style={{ fontSize: '1.5rem', fontWeight: 800, color: '#f8fafc' }}>{stats.totalResidents}</div>
            <div style={{ fontSize: '0.75rem', color: '#10b981', fontWeight: 600 }}>{stats.enrolledFaces} Đã quét mặt AI</div>
          </div>
        </div>

        <div className="glass-panel" style={{ padding: '20px', display: 'flex', alignItems: 'center', gap: '16px' }}>
          <div style={{ width: '48px', height: '48px', borderRadius: '12px', background: 'rgba(59, 130, 246, 0.15)', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
            <Building size={24} color="#3b82f6" />
          </div>
          <div>
            <div style={{ fontSize: '0.8rem', color: '#94a3b8', fontWeight: 600 }}>Tổng Căn Hộ Chung Cư</div>
            <div style={{ fontSize: '1.5rem', fontWeight: 800, color: '#f8fafc' }}>{stats.totalApartments}</div>
            <div style={{ fontSize: '0.75rem', color: '#64748b', fontWeight: 600 }}>12 Tòa tháp (10 tầng)</div>
          </div>
        </div>

        <div className="glass-panel" style={{ padding: '20px', display: 'flex', alignItems: 'center', gap: '16px' }}>
          <div style={{ width: '48px', height: '48px', borderRadius: '12px', background: 'rgba(16, 185, 129, 0.15)', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
            <DollarSign size={24} color="#10b981" />
          </div>
          <div>
            <div style={{ fontSize: '0.8rem', color: '#94a3b8', fontWeight: 600 }}>Doanh Thu Phí Dịch Vụ</div>
            <div style={{ fontSize: '1.3rem', fontWeight: 800, color: '#f8fafc' }}>
              {(stats.monthlyRevenue / 1000000).toFixed(1)} trđ
            </div>
            <div style={{ fontSize: '0.75rem', color: '#10b981', fontWeight: 600 }}>Tháng 08/2026</div>
          </div>
        </div>

        <div className="glass-panel" style={{ padding: '20px', display: 'flex', alignItems: 'center', gap: '16px' }}>
          <div style={{ width: '48px', height: '48px', borderRadius: '12px', background: 'rgba(244, 63, 94, 0.15)', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
            <ShieldAlert size={24} color="#f43f5e" />
          </div>
          <div>
            <div style={{ fontSize: '0.8rem', color: '#94a3b8', fontWeight: 600 }}>Cảnh Báo An Ninh</div>
            <div style={{ fontSize: '1.5rem', fontWeight: 800, color: '#f8fafc' }}>{alerts.length}</div>
            <div style={{ fontSize: '0.75rem', color: '#f43f5e', fontWeight: 600 }}>Phát hiện người lạ</div>
          </div>
        </div>
      </div>

      <div style={{ display: 'grid', gridTemplateColumns: '3fr 2fr', gap: '20px' }}>
        <div className="glass-panel" style={{ padding: '20px' }}>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '16px' }}>
            <h3 style={{ fontSize: '1.1rem', fontWeight: 700, color: '#f8fafc', display: 'flex', alignItems: 'center', gap: '8px' }}>
              <Elevator size={20} color="#06b6d4" /> Nhật Ký Ra Vào Thang Máy AI
            </h3>
            <button className="btn btn-secondary" style={{ fontSize: '0.8rem', padding: '6px 12px' }} onClick={() => onNavigate('security')}>
              Xem tất cả
            </button>
          </div>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
            {recentLogs.length === 0 ? (
              <div style={{ textAlign: 'center', padding: '30px', color: '#64748b' }}>Chưa có nhật ký ra vào nào</div>
            ) : (
              recentLogs.map(log => (
                <div key={log.id} style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '12px 16px', background: 'rgba(255,255,255,0.03)', borderRadius: '10px', border: '1px solid rgba(255,255,255,0.06)' }}>
                  <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
                    <div style={{ width: '36px', height: '36px', borderRadius: '50%', background: 'rgba(6, 182, 212, 0.2)', display: 'flex', alignItems: 'center', justifyContent: 'center', color: '#06b6d4', fontWeight: 700 }}>
                      T{log.floor}
                    </div>
                    <div>
                      <div style={{ fontSize: '0.9rem', fontWeight: 700, color: '#f8fafc' }}>
                        {log.resident_name || `Cư dân ID ${log.resident_id}`}
                      </div>
                      <div style={{ fontSize: '0.75rem', color: '#64748b' }}>
                        Căn hộ: {log.apartment || 'P502'} | Kích hoạt Tầng {log.floor}
                      </div>
                    </div>
                  </div>
                  <span className="badge badge-success">Thành Công</span>
                </div>
              ))
            )}
          </div>
        </div>

        <div className="glass-panel" style={{ padding: '20px' }}>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '16px' }}>
            <h3 style={{ fontSize: '1.1rem', fontWeight: 700, color: '#f8fafc', display: 'flex', alignItems: 'center', gap: '8px' }}>
              <ShieldAlert size={20} color="#f43f5e" /> Cảnh Báo An Ninh Realtime
            </h3>
          </div>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
            {alerts.length === 0 ? (
              <div style={{ textAlign: 'center', padding: '30px', color: '#10b981', display: 'flex', alignItems: 'center', justifyContent: 'center', gap: '8px' }}>
                <CheckCircle2 size={18} /> Hệ thống an toàn (0 Cảnh báo)
              </div>
            ) : (
              alerts.map(alert => (
                <div key={alert.id} style={{ padding: '12px 14px', background: 'rgba(244, 63, 94, 0.1)', borderRadius: '10px', border: '1px solid rgba(244, 63, 94, 0.3)' }}>
                  <div style={{ fontSize: '0.85rem', fontWeight: 700, color: '#f43f5e', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                    <span>{alert.reason}</span>
                    <span className="badge badge-danger">Cảnh báo</span>
                  </div>
                  <div style={{ fontSize: '0.75rem', color: '#94a3b8', marginTop: '4px' }}>
                    {new Date(alert.timestamp).toLocaleString('vi-VN')} | Thiết bị: {alert.device_id}
                  </div>
                </div>
              ))
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
