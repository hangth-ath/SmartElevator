import React from 'react';
import { LayoutDashboard, Users, Receipt, Wrench, ShieldAlert, Bell } from 'lucide-react';
import { ElevatorIcon as Elevator } from './ElevatorIcon';

interface SidebarProps {
  activeTab: string;
  setActiveTab: (tab: string) => void;
}

export const Sidebar: React.FC<SidebarProps> = ({ activeTab, setActiveTab }) => {
  const menuItems = [
    { id: 'dashboard', label: 'Tổng Quan', icon: LayoutDashboard },
    { id: 'residents', label: 'Quản Lý Cư Dân', icon: Users },
    { id: 'elevator', label: 'Giám Sát Thang Máy', icon: Elevator },
    { id: 'bills', label: 'Hóa Đơn & Phí', icon: Receipt },
    { id: 'maintenance', label: 'Phản Ánh Sửa Chữa', icon: Wrench },
    { id: 'security', label: 'Nhật Ký An Ninh', icon: ShieldAlert },
    { id: 'notifications', label: 'Thông Báo Bảng Tin', icon: Bell },
  ];

  return (
    <aside style={{
      width: '260px',
      background: 'rgba(15, 23, 42, 0.85)',
      backdropFilter: 'blur(20px)',
      borderRight: '1px solid rgba(255, 255, 255, 0.1)',
      display: 'flex',
      flexDirection: 'column',
      padding: '24px 16px',
      gap: '8px'
    }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: '12px', padding: '0 12px 24px 12px', borderBottom: '1px solid rgba(255,255,255,0.08)' }}>
        <div style={{
          width: '40px',
          height: '40px',
          borderRadius: '10px',
          background: 'linear-gradient(135deg, #06b6d4, #3b82f6)',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          boxShadow: '0 0 16px rgba(6, 182, 212, 0.5)'
        }}>
          <Elevator size={24} color="#ffffff" />
        </div>
        <div>
          <h2 style={{ fontSize: '1.1rem', fontWeight: 800, background: 'linear-gradient(to right, #06b6d4, #60a5fa)', WebkitBackgroundClip: 'text', WebkitTextFillColor: 'transparent' }}>
            SmartElevator
          </h2>
          <span style={{ fontSize: '0.7rem', color: '#64748b', fontWeight: 600, letterSpacing: '0.5px' }}>
            VNPT MEDIA MANAGEMENT
          </span>
        </div>
      </div>

      <nav style={{ display: 'flex', flexDirection: 'column', gap: '6px', marginTop: '16px' }}>
        {menuItems.map((item) => {
          const Icon = item.icon;
          const isActive = activeTab === item.id;
          return (
            <button
              key={item.id}
              onClick={() => setActiveTab(item.id)}
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: '12px',
                padding: '12px 16px',
                borderRadius: '12px',
                border: 'none',
                background: isActive ? 'linear-gradient(135deg, rgba(6, 182, 212, 0.2), rgba(59, 130, 246, 0.2))' : 'transparent',
                color: isActive ? '#06b6d4' : '#94a3b8',
                fontWeight: isActive ? 700 : 500,
                fontSize: '0.9rem',
                cursor: 'pointer',
                transition: 'all 0.2s ease',
                borderLeft: isActive ? '3px solid #06b6d4' : '3px solid transparent'
              }}
            >
              <Icon size={18} color={isActive ? '#06b6d4' : '#94a3b8'} />
              <span>{item.label}</span>
            </button>
          );
        })}
      </nav>

      <div style={{ marginTop: 'auto', padding: '12px', background: 'rgba(255,255,255,0.03)', borderRadius: '12px', border: '1px solid rgba(255,255,255,0.06)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <span style={{ width: '8px', height: '8px', borderRadius: '50%', background: '#10b981', boxShadow: '0 0 8px #10b981' }}></span>
          <span style={{ fontSize: '0.75rem', fontWeight: 600, color: '#f8fafc' }}>Thiết Bị C++ Biên Online</span>
        </div>
        <div style={{ fontSize: '0.7rem', color: '#64748b', marginTop: '4px' }}>IP: localhost:8080 (COM3)</div>
      </div>
    </aside>
  );
};
