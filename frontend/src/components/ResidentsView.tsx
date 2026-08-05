import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Users, UserPlus, Camera, Trash2, CheckCircle2, AlertCircle, RefreshCw, X, Sparkles } from 'lucide-react';
import { ElevatorIcon as Elevator } from './ElevatorIcon';
import { Resident, DeviceStatus } from '../types';

export const ResidentsView: React.FC = () => {
  const [residents, setResidents] = useState<Resident[]>([]);
  const [loading, setLoading] = useState<boolean>(true);
  const [search, setSearch] = useState<string>('');

  const [isAddModalOpen, setIsAddModalOpen] = useState<boolean>(false);
  const [formData, setFormData] = useState({ name: '', apartment: '', target_floor: 1 });

  const [enrollingResident, setEnrollingResident] = useState<Resident | null>(null);
  const [enrollStatus, setEnrollStatus] = useState<DeviceStatus | null>(null);
  const [enrollError, setEnrollError] = useState<string | null>(null);
  const [isEnrollingBusy, setIsEnrollingBusy] = useState<boolean>(false);
  const [isEnrollSuccess, setIsEnrollSuccess] = useState<boolean>(false);

  const fetchResidents = () => {
    setLoading(true);
    axios.get('/api/residents')
      .then(res => {
        setResidents(res.data);
        setLoading(false);
      })
      .catch(err => {
        console.error(err);
        setLoading(false);
      });
  };

  useEffect(() => {
    fetchResidents();
  }, []);

  const handleAddResident = (e: React.FormEvent) => {
    e.preventDefault();
    if (!formData.name || !formData.apartment) return;

    axios.post('/api/residents', formData)
      .then(res => {
        const newResident: Resident = res.data;
        setIsAddModalOpen(false);
        setFormData({ name: '', apartment: '', target_floor: 1 });
        fetchResidents();
        startFaceEnrollment(newResident);
      })
      .catch(err => alert('Lỗi tạo cư dân: ' + err.message));
  };

  const startFaceEnrollment = (resident: Resident) => {
    setEnrollingResident(resident);
    setEnrollError(null);
    setIsEnrollingBusy(false);
    setIsEnrollSuccess(false);
    setEnrollStatus({ mode: 'ENROLLING', enrollProgress: 0, elevatorState: 'IDLE', currentFloor: 1, targetFloor: resident.target_floor });

    axios.post(`/api/residents/${resident.id}/start-enrollment`)
      .then(res => {
        console.log('[FRONTEND] Đã phát lệnh enrollment cho C++ Device:', res.data);
      })
      .catch(err => {
        if (err.response && err.response.status === 409) {
          setIsEnrollingBusy(true);
          setEnrollError('Thiết bị đang bận đăng ký cư dân khác, vui lòng thử lại sau.');
        } else {
          setEnrollError(err.response?.data?.error || 'Không thể kết nối với thiết bị C++ SmartElevator');
        }
      });
  };

  useEffect(() => {
    if (!enrollingResident || isEnrollingBusy || isEnrollSuccess) return;

    const interval = setInterval(() => {
      axios.get(`/api/residents/${enrollingResident.id}/enrollment-status`)
        .then(res => {
          const status: DeviceStatus = res.data;
          setEnrollStatus(status);

          if (status.mode === 'RECOGNIZING' && status.enrollProgress >= 30) {
            setIsEnrollSuccess(true);
            fetchResidents();
            clearInterval(interval);
          }
        })
        .catch(console.error);
    }, 1000);

    return () => clearInterval(interval);
  }, [enrollingResident, isEnrollingBusy, isEnrollSuccess]);

  const handleCancelEnrollment = () => {
    axios.post('/device/cancel-enrollment').catch(() => {});
    setEnrollingResident(null);
    setEnrollStatus(null);
  };

  const handleDeleteResident = (id: number) => {
    if (!confirm('Bạn có chắc chắn muốn xóa cư dân này?')) return;
    axios.delete(`/api/residents/${id}`)
      .then(() => fetchResidents())
      .catch(err => alert('Lỗi xóa cư dân: ' + err.message));
  };

  const filteredResidents = residents.filter(r =>
    r.name.toLowerCase().includes(search.toLowerCase()) ||
    r.apartment.toLowerCase().includes(search.toLowerCase())
  );

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
      <div className="glass-panel" style={{ padding: '16px 24px', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '16px', flex: 1, maxWidth: '400px' }}>
          <input
            type="text"
            className="input-field"
            placeholder="Tìm kiếm theo Tên hoặc Số Phòng..."
            value={search}
            onChange={e => setSearch(e.target.value)}
          />
        </div>

        <button className="btn btn-primary" onClick={() => setIsAddModalOpen(true)}>
          <UserPlus size={18} /> Thêm Cư Dân Mới (2 Bước)
        </button>
      </div>

      <div className="glass-panel" style={{ overflow: 'hidden' }}>
        <table style={{ width: '100%', borderCollapse: 'collapse', textAlign: 'left' }}>
          <thead>
            <tr style={{ background: 'rgba(255,255,255,0.04)', borderBottom: '1px solid rgba(255,255,255,0.08)', color: '#94a3b8', fontSize: '0.8rem', textTransform: 'uppercase' }}>
              <th style={{ padding: '14px 20px' }}>ID</th>
              <th style={{ padding: '14px 20px' }}>Họ và Tên</th>
              <th style={{ padding: '14px 20px' }}>Căn Hộ</th>
              <th style={{ padding: '14px 20px' }}>Tầng Đích</th>
              <th style={{ padding: '14px 20px' }}>Dữ Liệu Khuôn Mặt</th>
              <th style={{ padding: '14px 20px', textAlign: 'right' }}>Hành Động</th>
            </tr>
          </thead>
          <tbody>
            {loading ? (
              <tr><td colSpan={6} style={{ textAlign: 'center', padding: '40px', color: '#64748b' }}>Đang tải dữ liệu cư dân...</td></tr>
            ) : filteredResidents.length === 0 ? (
              <tr><td colSpan={6} style={{ textAlign: 'center', padding: '40px', color: '#64748b' }}>Không tìm thấy cư dân nào</td></tr>
            ) : (
              filteredResidents.map(r => (
                <tr key={r.id} style={{ borderBottom: '1px solid rgba(255,255,255,0.05)', transition: 'background 0.2s' }}>
                  <td style={{ padding: '14px 20px', fontWeight: 700, color: '#64748b' }}>#{r.id}</td>
                  <td style={{ padding: '14px 20px', fontWeight: 700, color: '#f8fafc' }}>{r.name}</td>
                  <td style={{ padding: '14px 20px', color: '#06b6d4', fontWeight: 700 }}>{r.apartment}</td>
                  <td style={{ padding: '14px 20px', color: '#f8fafc' }}>Tầng {r.target_floor}</td>
                  <td style={{ padding: '14px 20px' }}>
                    {r.face_enrolled === 1 ? (
                      <span className="badge badge-success"><CheckCircle2 size={12} /> Đã Đăng Ký AI</span>
                    ) : (
                      <span className="badge badge-warning"><AlertCircle size={12} /> Chưa Có Khuôn Mặt</span>
                    )}
                  </td>
                  <td style={{ padding: '14px 20px', textAlign: 'right' }}>
                    <div style={{ display: 'flex', gap: '8px', justifyContent: 'flex-end' }}>
                      <button
                        className="btn btn-secondary"
                        style={{ padding: '6px 12px', fontSize: '0.8rem', background: 'rgba(6, 182, 212, 0.15)', color: '#06b6d4', border: '1px solid rgba(6,182,212,0.3)' }}
                        onClick={() => startFaceEnrollment(r)}
                      >
                        <Camera size={14} /> Quét Mặt AI
                      </button>
                      <button
                        className="btn btn-danger"
                        style={{ padding: '6px 10px' }}
                        onClick={() => handleDeleteResident(r.id)}
                      >
                        <Trash2 size={14} />
                      </button>
                    </div>
                  </td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>

      {isAddModalOpen && (
        <div style={{ position: 'fixed', inset: 0, background: 'rgba(0,0,0,0.7)', display: 'flex', alignItems: 'center', justifyContent: 'center', zIndex: 100 }}>
          <div className="glass-modal" style={{ width: '450px', borderRadius: '16px', padding: '24px', position: 'relative' }}>
            <button onClick={() => setIsAddModalOpen(false)} style={{ position: 'absolute', top: '16px', right: '16px', background: 'none', border: 'none', color: '#94a3b8', cursor: 'pointer' }}>
              <X size={20} />
            </button>
            <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#f8fafc', marginBottom: '8px' }}>
              Bước 1: Nhập Thông Tin Cư Dân Mới
            </h3>
            <p style={{ fontSize: '0.85rem', color: '#94a3b8', marginBottom: '20px' }}>
              Nhập thông tin cư dân để tạo hồ sơ lưu trong CSDL trung tâm.
            </p>

            <form onSubmit={handleAddResident} style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
              <div>
                <label style={{ fontSize: '0.8rem', fontWeight: 600, color: '#94a3b8', display: 'block', marginBottom: '6px' }}>Họ và Tên Cư Dân</label>
                <input
                  type="text"
                  className="input-field"
                  placeholder="Ví dụ: Nguyễn Văn A"
                  value={formData.name}
                  onChange={e => setFormData({ ...formData, name: e.target.value })}
                  required
                />
              </div>

              <div>
                <label style={{ fontSize: '0.8rem', fontWeight: 600, color: '#94a3b8', display: 'block', marginBottom: '6px' }}>Mã Số Phòng Căn Hộ (Chuỗi)</label>
                <input
                  type="text"
                  className="input-field"
                  placeholder="Ví dụ: P502, P804, P1005"
                  value={formData.apartment}
                  onChange={e => setFormData({ ...formData, apartment: e.target.value })}
                  required
                />
              </div>

              <div>
                <label style={{ fontSize: '0.8rem', fontWeight: 600, color: '#94a3b8', display: 'block', marginBottom: '6px' }}>Tầng Đích Thang Máy (1-10)</label>
                <input
                  type="number"
                  min={1}
                  max={10}
                  className="input-field"
                  value={formData.target_floor}
                  onChange={e => setFormData({ ...formData, target_floor: parseInt(e.target.value) })}
                  required
                />
              </div>

              <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '12px', marginTop: '12px' }}>
                <button type="button" className="btn btn-secondary" onClick={() => setIsAddModalOpen(false)}>Hủy</button>
                <button type="submit" className="btn btn-primary">Tiếp Theo: Quét Khuôn Mặt <Sparkles size={16} /></button>
              </div>
            </form>
          </div>
        </div>
      )}

      {enrollingResident && (
        <div style={{ position: 'fixed', inset: 0, background: 'rgba(0,0,0,0.85)', display: 'flex', alignItems: 'center', justifyContent: 'center', zIndex: 110 }}>
          <div className="glass-modal" style={{ width: '500px', borderRadius: '20px', padding: '32px', textAlign: 'center', position: 'relative' }}>
            <button onClick={handleCancelEnrollment} style={{ position: 'absolute', top: '16px', right: '16px', background: 'none', border: 'none', color: '#94a3b8', cursor: 'pointer' }}>
              <X size={20} />
            </button>

            {isEnrollingBusy ? (
              <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '16px' }}>
                <div style={{ width: '64px', height: '64px', borderRadius: '50%', background: 'rgba(245, 158, 11, 0.2)', display: 'flex', alignItems: 'center', justifyContent: 'center', color: '#f59e0b' }}>
                  <AlertCircle size={36} />
                </div>
                <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#f59e0b' }}>Thiết Bị Đang Bận!</h3>
                <p style={{ color: '#94a3b8', fontSize: '0.9rem' }}>{enrollError}</p>
                <button className="btn btn-primary" onClick={() => startFaceEnrollment(enrollingResident)}>
                  <RefreshCw size={16} /> Bấm Thử Lại
                </button>
              </div>
            ) : isEnrollSuccess ? (
              <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '16px' }}>
                <div style={{ width: '64px', height: '64px', borderRadius: '50%', background: 'rgba(16, 185, 129, 0.2)', display: 'flex', alignItems: 'center', justifyContent: 'center', color: '#10b981' }}>
                  <CheckCircle2 size={36} />
                </div>
                <h3 style={{ fontSize: '1.3rem', fontWeight: 800, color: '#10b981' }}>Đăng Ký Khuôn Mặt Thành Công!</h3>
                <p style={{ color: '#94a3b8', fontSize: '0.9rem' }}>
                  Đã thu thập đủ 30 mẫu ảnh cho cư dân <strong>{enrollingResident.name}</strong> (Phòng {enrollingResident.apartment} - Tầng {enrollingResident.target_floor}).
                </p>
                <div style={{ padding: '12px 16px', background: 'rgba(255,255,255,0.05)', borderRadius: '10px', fontSize: '0.85rem', color: '#06b6d4', width: '100%' }}>
                  Trạng Thái Thiết Bị C++: <strong>{enrollStatus?.elevatorState || 'IDLE'}</strong> (Đã cập nhật Mô hình LBPH AI)
                </div>
                <button className="btn btn-primary" onClick={() => setEnrollingResident(null)}>
                  Hoàn Tất
                </button>
              </div>
            ) : (
              <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '16px' }}>
                <div style={{ width: '64px', height: '64px', borderRadius: '50%', background: 'linear-gradient(135deg, #06b6d4, #3b82f6)', display: 'flex', alignItems: 'center', justifyContent: 'center', color: '#fff' }} className="animate-glow">
                  <Camera size={32} />
                </div>

                <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#f8fafc' }}>
                  Đang Quét Khuôn Mặt Cư Dân
                </h3>
                <p style={{ color: '#06b6d4', fontWeight: 700, fontSize: '1.1rem' }}>
                  {enrollingResident.name} ({enrollingResident.apartment} - Tầng {enrollingResident.target_floor})
                </p>
                <p style={{ color: '#64748b', fontSize: '0.85rem' }}>
                  Mời cư dân nhìn trực diện vào Camera của máy tính C++ SmartElevator.
                </p>

                <div style={{ width: '100%', marginTop: '12px' }}>
                  <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.85rem', color: '#94a3b8', marginBottom: '8px', fontWeight: 600 }}>
                    <span>Tiến độ chụp mẫu:</span>
                    <span style={{ color: '#06b6d4' }}>{enrollStatus?.enrollProgress || 0} / 30 Mẫu</span>
                  </div>
                  <div style={{ width: '100%', height: '12px', background: 'rgba(255,255,255,0.1)', borderRadius: '6px', overflow: 'hidden' }}>
                    <div style={{
                      width: `${Math.min(100, ((enrollStatus?.enrollProgress || 0) / 30) * 100)}%`,
                      height: '100%',
                      background: 'linear-gradient(90deg, #06b6d4, #10b981)',
                      transition: 'width 0.3s ease'
                    }}></div>
                  </div>
                </div>

                <div style={{ display: 'flex', alignItems: 'center', gap: '8px', marginTop: '12px', color: '#94a3b8', fontSize: '0.8rem' }}>
                  <RefreshCw size={14} className="spin" /> Đang poll kết quả từ HTTP Server C++ (port 8080)...
                </div>

                <button className="btn btn-secondary" style={{ marginTop: '16px' }} onClick={handleCancelEnrollment}>
                  Hủy Đăng Ký
                </button>
              </div>
            )}
          </div>
        </div>
      )}
    </div>
  );
};
