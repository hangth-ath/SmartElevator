import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Bell, Plus, Megaphone, Calendar, User } from 'lucide-react';
import { NotificationItem } from '../types';

export const NotificationsView: React.FC = () => {
  const [notifications, setNotifications] = useState<NotificationItem[]>([]);
  const [showModal, setShowModal] = useState<boolean>(false);
  const [formData, setFormData] = useState({ title: '', content: '' });

  const fetchNotifications = () => {
    axios.get('/api/notifications').then(res => setNotifications(res.data)).catch(console.error);
  };

  useEffect(() => {
    fetchNotifications();
  }, []);

  const handleCreate = (e: React.FormEvent) => {
    e.preventDefault();
    if (!formData.title || !formData.content) return;
    axios.post('/api/notifications', formData)
      .then(() => {
        setShowModal(false);
        setFormData({ title: '', content: '' });
        fetchNotifications();
      })
      .catch(err => alert('Lỗi tạo thông báo: ' + err.message));
  };

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
      <div className="glass-panel" style={{ padding: '20px 24px', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <div>
          <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#f8fafc' }}>Thông Báo & Bảng Tin Chung Cư</h3>
          <p style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Đăng tải các thông báo bảo trì, lịch đóng phí tới cư dân.</p>
        </div>
        <button className="btn btn-primary" onClick={() => setShowModal(true)}>
          <Plus size={18} /> Tạo Thông Báo Mới
        </button>
      </div>

      <div style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
        {notifications.map(n => (
          <div key={n.id} className="glass-panel" style={{ padding: '20px', borderLeft: '4px solid #06b6d4' }}>
            <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '8px' }}>
              <h4 style={{ fontSize: '1.1rem', fontWeight: 700, color: '#f8fafc', display: 'flex', alignItems: 'center', gap: '8px' }}>
                <Megaphone size={18} color="#06b6d4" /> {n.title}
              </h4>
              <span className="badge badge-info">{n.scope}</span>
            </div>
            <p style={{ color: '#cbd5e1', fontSize: '0.9rem', marginBottom: '12px', lineHeight: 1.6 }}>{n.content}</p>
            <div style={{ display: 'flex', gap: '16px', fontSize: '0.75rem', color: '#64748b' }}>
              <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}><User size={12} /> {n.created_by}</span>
              <span style={{ display: 'flex', alignItems: 'center', gap: '4px' }}><Calendar size={12} /> {new Date(n.created_at).toLocaleString('vi-VN')}</span>
            </div>
          </div>
        ))}
      </div>

      {showModal && (
        <div style={{ position: 'fixed', inset: 0, background: 'rgba(0,0,0,0.7)', display: 'flex', alignItems: 'center', justifyContent: 'center', zIndex: 100 }}>
          <div className="glass-modal" style={{ width: '450px', padding: '24px', borderRadius: '16px' }}>
            <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#f8fafc', marginBottom: '16px' }}>Tạo Thông Báo Mới</h3>
            <form onSubmit={handleCreate} style={{ display: 'flex', flexDirection: 'column', gap: '14px' }}>
              <input
                type="text"
                className="input-field"
                placeholder="Tiêu đề thông báo..."
                value={formData.title}
                onChange={e => setFormData({ ...formData, title: e.target.value })}
                required
              />
              <textarea
                className="input-field"
                rows={4}
                placeholder="Nội dung thông báo..."
                value={formData.content}
                onChange={e => setFormData({ ...formData, content: e.target.value })}
                required
              />
              <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px' }}>
                <button type="button" className="btn btn-secondary" onClick={() => setShowModal(false)}>Hủy</button>
                <button type="submit" className="btn btn-primary">Đăng Thông Báo</button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
};
