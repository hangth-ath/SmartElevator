import React from 'react';
import { Bell, UserCheck, Shield } from 'lucide-react';

interface HeaderProps {
  title: string;
}

export const Header: React.FC<HeaderProps> = ({ title }) => {
  return (
    <header style={{
      height: '70px',
      background: 'rgba(15, 23, 42, 0.6)',
      backdropFilter: 'blur(16px)',
      borderBottom: '1px solid rgba(255, 255, 255, 0.08)',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'space-between',
      padding: '0 32px'
    }}>
      <div>
        <h1 style={{ fontSize: '1.4rem', fontWeight: 800, color: '#f8fafc', letterSpacing: '-0.3px' }}>
          {title}
        </h1>
        <p style={{ fontSize: '0.8rem', color: '#64748b' }}>Hệ Thống Quản Lý Chung Cư AI Tích Hợp Camera Thang Máy C++</p>
      </div>

      <div style={{ display: 'flex', alignItems: 'center', gap: '16px' }}>
        {/* Status Badge */}
        <div style={{
          display: 'flex',
          alignItems: 'center',
          gap: '8px',
          padding: '6px 12px',
          background: 'rgba(16, 185, 129, 0.1)',
          border: '1px solid rgba(16, 185, 129, 0.3)',
          borderRadius: '20px'
        }}>
          <Shield size={14} color="#10b981" />
          <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#10b981' }}>HỆ THỐNG AN NINH KÍCH HOẠT</span>
        </div>

        {/* Notifications Icon */}
        <button style={{
          width: '38px',
          height: '38px',
          borderRadius: '10px',
          background: 'rgba(255, 255, 255, 0.05)',
          border: '1px solid rgba(255, 255, 255, 0.1)',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          cursor: 'pointer'
        }}>
          <Bell size={18} color="#94a3b8" />
        </button>

        {/* User Profile */}
        <div style={{ display: 'flex', alignItems: 'center', gap: '10px', paddingLeft: '12px', borderLeft: '1px solid rgba(255,255,255,0.1)' }}>
          <div style={{
            width: '36px',
            height: '36px',
            borderRadius: '50%',
            background: 'linear-gradient(135deg, #3b82f6, #8b5cf6)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            fontWeight: 700,
            fontSize: '0.9rem',
            color: '#ffffff'
          }}>
            <UserCheck size={18} />
          </div>
          <div>
            <div style={{ fontSize: '0.85rem', fontWeight: 700, color: '#f8fafc' }}>Ban Quản Trị</div>
            <div style={{ fontSize: '0.7rem', color: '#64748b' }}>Admin System</div>
          </div>
        </div>
      </div>
    </header>
  );
};
