import React, { useState } from 'react';
import { Sidebar } from './components/Sidebar';
import { Header } from './components/Header';
import { DashboardView } from './components/DashboardView';
import { ResidentsView } from './components/ResidentsView';
import { ElevatorMonitorView } from './components/ElevatorMonitorView';
import { BillsView } from './components/BillsView';
import { MaintenanceView } from './components/MaintenanceView';
import { AccessLogsView } from './components/AccessLogsView';
import { NotificationsView } from './components/NotificationsView';

export const App: React.FC = () => {
  const [activeTab, setActiveTab] = useState<string>('dashboard');

  const renderContent = () => {
    switch (activeTab) {
      case 'dashboard': return <DashboardView onNavigate={setActiveTab} />;
      case 'residents': return <ResidentsView />;
      case 'elevator': return <ElevatorMonitorView />;
      case 'bills': return <BillsView />;
      case 'maintenance': return <MaintenanceView />;
      case 'security': return <AccessLogsView />;
      case 'notifications': return <NotificationsView />;
      default: return <DashboardView onNavigate={setActiveTab} />;
    }
  };

  const getTitle = () => {
    switch (activeTab) {
      case 'dashboard': return 'Tổng Quan Hệ Thống';
      case 'residents': return 'Quản Lý Cư Dân & Đăng Ký Khuôn Mặt AI';
      case 'elevator': return 'Giám Sát Thang Máy Trực Tiếp';
      case 'bills': return 'Quản Lý Hóa Đơn & Thanh Toán';
      case 'maintenance': return 'Phản Ánh & Sửa Chữa Kỹ Thuật';
      case 'security': return 'Nhật Ký Truy Cập AI & Cảnh Báo An Ninh';
      case 'notifications': return 'Bảng Tin & Thông Báo';
      default: return 'SmartElevator';
    }
  };

  return (
    <div style={{ display: 'flex', minHeight: '100vh', width: '100vw' }}>
      <Sidebar activeTab={activeTab} setActiveTab={setActiveTab} />
      <div style={{ flex: 1, display: 'flex', flexDirection: 'column', overflowX: 'hidden' }}>
        <Header title={getTitle()} />
        <main style={{ flex: 1, padding: '32px', overflowY: 'auto' }}>
          {renderContent()}
        </main>
      </div>
    </div>
  );
};

export default App;
