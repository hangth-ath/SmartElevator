import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { ArrowUp, ArrowDown, Shield, RefreshCw } from 'lucide-react';
import { ElevatorIcon as Elevator } from './ElevatorIcon';
import { DeviceStatus } from '../types';

export const ElevatorMonitorView: React.FC = () => {
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus>({
    mode: 'RECOGNIZING',
    enrollProgress: 0,
    elevatorState: 'IDLE',
    currentFloor: 1,
    targetFloor: 1
  });

  const [simulatedFloor, setSimulatedFloor] = useState<number>(1);

  useEffect(() => {
    const interval = setInterval(() => {
      axios.get('/api/residents/1/enrollment-status')
        .then(res => {
          setDeviceStatus(res.data);
          if (res.data.currentFloor) {
            setSimulatedFloor(res.data.currentFloor);
          }
        })
        .catch(() => {});
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '24px' }}>
      <div className="glass-panel" style={{ padding: '24px', display: 'flex', flexDirection: 'column', alignItems: 'center', background: 'radial-gradient(ellipse at center, rgba(15,23,42,0.9), rgba(10,15,28,0.95))' }}>
        <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#06b6d4', marginBottom: '16px', display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Elevator size={24} /> Giao Diện Cabin & LED Thang Máy Live
        </h3>

        <div style={{
          width: '180px',
          height: '90px',
          background: '#090d16',
          border: '2px solid rgba(6, 182, 212, 0.5)',
          borderRadius: '12px',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          gap: '12px',
          boxShadow: '0 0 20px rgba(6, 182, 212, 0.3)',
          marginBottom: '24px'
        }}>
          <span style={{ fontSize: '3rem', fontWeight: 800, color: '#10b981', fontFamily: 'monospace' }}>
            {simulatedFloor < 10 ? `0${simulatedFloor}` : simulatedFloor}
          </span>
          {deviceStatus.targetFloor > simulatedFloor ? (
            <ArrowUp size={32} color="#06b6d4" className="animate-glow" />
          ) : deviceStatus.targetFloor < simulatedFloor ? (
            <ArrowDown size={32} color="#f43f5e" className="animate-glow" />
          ) : (
            <div style={{ width: '12px', height: '12px', borderRadius: '50%', background: '#10b981' }}></div>
          )}
        </div>

        <div style={{ width: '100%', height: '320px', background: 'rgba(255,255,255,0.02)', borderRadius: '16px', border: '1px solid rgba(255,255,255,0.08)', position: 'relative', overflow: 'hidden', padding: '16px' }}>
          {Array.from({ length: 10 }, (_, i) => 10 - i).map(floorNum => (
            <div key={floorNum} style={{
              height: '28px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
              borderBottom: '1px dashed rgba(255,255,255,0.06)',
              fontSize: '0.75rem',
              color: floorNum === simulatedFloor ? '#06b6d4' : '#64748b',
              fontWeight: floorNum === simulatedFloor ? 800 : 500
            }}>
              <span>Tầng {floorNum}</span>
              {floorNum === simulatedFloor && (
                <span className="badge badge-info" style={{ fontSize: '0.65rem' }}>Cabin ở đây</span>
              )}
            </div>
          ))}
        </div>
      </div>

      <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
        <div className="glass-panel" style={{ padding: '24px' }}>
          <h3 style={{ fontSize: '1.1rem', fontWeight: 800, color: '#f8fafc', marginBottom: '16px', display: 'flex', alignItems: 'center', gap: '8px' }}>
            <Shield size={20} color="#10b981" /> Thông Số Thiết Bị C++ Biên Realtime
          </h3>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '12px', background: 'rgba(255,255,255,0.03)', borderRadius: '10px' }}>
              <span style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Chế độ hoạt động (Device Mode):</span>
              <span className={`badge ${deviceStatus.mode === 'ENROLLING' ? 'badge-warning' : 'badge-success'}`}>
                {deviceStatus.mode}
              </span>
            </div>

            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '12px', background: 'rgba(255,255,255,0.03)', borderRadius: '10px' }}>
              <span style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Trạng thái thang (State Machine):</span>
              <span style={{ fontWeight: 700, color: '#06b6d4' }}>{deviceStatus.elevatorState}</span>
            </div>

            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '12px', background: 'rgba(255,255,255,0.03)', borderRadius: '10px' }}>
              <span style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Tầng hiện tại (Current Floor):</span>
              <span style={{ fontWeight: 700, color: '#f8fafc' }}>Tầng {deviceStatus.currentFloor}</span>
            </div>

            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '12px', background: 'rgba(255,255,255,0.03)', borderRadius: '10px' }}>
              <span style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Tầng kích hoạt (Target Floor):</span>
              <span style={{ fontWeight: 700, color: '#10b981' }}>Tầng {deviceStatus.targetFloor}</span>
            </div>

            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '12px', background: 'rgba(255,255,255,0.03)', borderRadius: '10px' }}>
              <span style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Cổng phần cứng (Hardware Port):</span>
              <span style={{ fontWeight: 700, color: '#64748b' }}>COM3 (9600 Baud)</span>
            </div>
          </div>
        </div>

        <div className="glass-panel" style={{ padding: '24px' }}>
          <h3 style={{ fontSize: '1.1rem', fontWeight: 800, color: '#f8fafc', marginBottom: '12px' }}>
            Thử Nghiệm Kích Hoạt Thang Máy Giả Lập
          </h3>
          <p style={{ color: '#94a3b8', fontSize: '0.85rem', marginBottom: '16px' }}>
            Chọn tầng bên dưới để đưa tín hiệu mô phỏng tới hệ thống nhận diện:
          </p>

          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(5, 1fr)', gap: '10px' }}>
            {Array.from({ length: 10 }, (_, i) => i + 1).map(f => (
              <button
                key={f}
                className="btn btn-secondary"
                style={{ justifyContent: 'center', background: simulatedFloor === f ? '#06b6d4' : 'rgba(255,255,255,0.05)', color: simulatedFloor === f ? '#fff' : '#94a3b8' }}
                onClick={() => setSimulatedFloor(f)}
              >
                Tầng {f}
              </button>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
};
