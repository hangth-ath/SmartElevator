import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Wrench, Clock, CheckCircle2, AlertTriangle, Plus, User } from 'lucide-react';
import { MaintenanceRequest } from '../types';

export const MaintenanceView: React.FC = () => {
  const [requests, setRequests] = useState<MaintenanceRequest[]>([]);
  const [loading, setLoading] = useState<boolean>(true);

  const fetchRequests = () => {
    setLoading(true);
    axios.get('/api/maintenance-requests').then(res => {
      setRequests(res.data);
      setLoading(false);
    }).catch(console.error);
  };

  useEffect(() => {
    fetchRequests();
  }, []);

  const updateStatus = (id: number, status: 'PENDING' | 'IN_PROGRESS' | 'COMPLETED') => {
    axios.put(`/api/maintenance-requests/${id}`, { status, assigned_to: 'Kỹ thuật viên Trưởng' })
      .then(() => fetchRequests())
      .catch(err => alert('Lỗi cập nhật: ' + err.message));
  };

  const pendingList = requests.filter(r => r.status === 'PENDING');
  const inProgressList = requests.filter(r => r.status === 'IN_PROGRESS');
  const completedList = requests.filter(r => r.status === 'COMPLETED');

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
      <div className="glass-panel" style={{ padding: '20px 24px', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <div>
          <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#f8fafc' }}>Quản Lý Phản Ánh & Sửa Chữa Kỹ Thuật (Kanban Board)</h3>
          <p style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Bảng theo dõi tiến độ xử lý phản ánh kỹ thuật điện nước, thang máy từ cư dân.</p>
        </div>
      </div>

      {/* Kanban Columns Grid */}
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(3, 1fr)', gap: '20px' }}>
        {/* Column 1: PENDING */}
        <div className="glass-panel" style={{ padding: '16px', background: 'rgba(30, 41, 59, 0.5)' }}>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '16px', paddingBottom: '10px', borderBottom: '2px solid rgba(245, 158, 11, 0.5)' }}>
            <h4 style={{ color: '#f59e0b', fontWeight: 800, fontSize: '0.95rem', display: 'flex', alignItems: 'center', gap: '8px' }}>
              <Clock size={16} /> CHỜ TIẾP NHẬN ({pendingList.length})
            </h4>
          </div>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
            {pendingList.map(item => (
              <div key={item.id} style={{ padding: '14px', background: 'rgba(15,23,42,0.8)', borderRadius: '10px', border: '1px solid rgba(255,255,255,0.08)' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '6px' }}>
                  <span style={{ fontWeight: 700, color: '#06b6d4', fontSize: '0.85rem' }}>{item.apartment} - {item.resident_name}</span>
                  <span className="badge badge-warning">{item.priority}</span>
                </div>
                <div style={{ fontWeight: 700, color: '#f8fafc', fontSize: '0.9rem', marginBottom: '4px' }}>{item.title}</div>
                <div style={{ color: '#94a3b8', fontSize: '0.8rem', marginBottom: '12px' }}>{item.description}</div>
                <button className="btn btn-primary" style={{ width: '100%', justifyContent: 'center', fontSize: '0.8rem', padding: '6px' }} onClick={() => updateStatus(item.id, 'IN_PROGRESS')}>
                  Tiếp Nhận Xử Lý &rarr;
                </button>
              </div>
            ))}
          </div>
        </div>

        {/* Column 2: IN_PROGRESS */}
        <div className="glass-panel" style={{ padding: '16px', background: 'rgba(30, 41, 59, 0.5)' }}>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '16px', paddingBottom: '10px', borderBottom: '2px solid rgba(6, 182, 212, 0.5)' }}>
            <h4 style={{ color: '#06b6d4', fontWeight: 800, fontSize: '0.95rem', display: 'flex', alignItems: 'center', gap: '8px' }}>
              <Wrench size={16} /> ĐANG SỬA CHỮA ({inProgressList.length})
            </h4>
          </div>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
            {inProgressList.map(item => (
              <div key={item.id} style={{ padding: '14px', background: 'rgba(15,23,42,0.8)', borderRadius: '10px', border: '1px solid rgba(6, 182, 212, 0.3)' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '6px' }}>
                  <span style={{ fontWeight: 700, color: '#06b6d4', fontSize: '0.85rem' }}>{item.apartment} - {item.resident_name}</span>
                  <span className="badge badge-info">Đang sửa</span>
                </div>
                <div style={{ fontWeight: 700, color: '#f8fafc', fontSize: '0.9rem', marginBottom: '4px' }}>{item.title}</div>
                <div style={{ color: '#94a3b8', fontSize: '0.8rem', marginBottom: '12px' }}>{item.description}</div>
                <button className="btn btn-primary" style={{ width: '100%', justifyContent: 'center', fontSize: '0.8rem', padding: '6px', background: 'linear-gradient(135deg, #10b981, #059669)' }} onClick={() => updateStatus(item.id, 'COMPLETED')}>
                  <CheckCircle2 size={14} /> Hoàn Thành Sửa Chữa
                </button>
              </div>
            ))}
          </div>
        </div>

        {/* Column 3: COMPLETED */}
        <div className="glass-panel" style={{ padding: '16px', background: 'rgba(30, 41, 59, 0.5)' }}>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '16px', paddingBottom: '10px', borderBottom: '2px solid rgba(16, 185, 129, 0.5)' }}>
            <h4 style={{ color: '#10b981', fontWeight: 800, fontSize: '0.95rem', display: 'flex', alignItems: 'center', gap: '8px' }}>
              <CheckCircle2 size={16} /> ĐÃ HOÀN THÀNH ({completedList.length})
            </h4>
          </div>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
            {completedList.map(item => (
              <div key={item.id} style={{ padding: '14px', background: 'rgba(15,23,42,0.8)', borderRadius: '10px', border: '1px solid rgba(16, 185, 129, 0.3)', opacity: 0.85 }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '6px' }}>
                  <span style={{ fontWeight: 700, color: '#10b981', fontSize: '0.85rem' }}>{item.apartment}</span>
                  <span className="badge badge-success">Done</span>
                </div>
                <div style={{ fontWeight: 700, color: '#f8fafc', fontSize: '0.9rem' }}>{item.title}</div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
};
