export interface Resident {
  id: number;
  name: string;
  apartment: string;
  target_floor: number;
  face_enrolled: number; // 0 or 1
  created_at?: string;
}

export interface Bill {
  id: number;
  apartment: string;
  period: string;
  amount: number;
  due_date: string;
  status: 'PAID' | 'UNPAID';
}

export interface NotificationItem {
  id: number;
  title: string;
  content: string;
  scope: string;
  created_by: string;
  created_at: string;
}

export interface MaintenanceRequest {
  id: number;
  resident_id: number;
  resident_name: string;
  apartment: string;
  title: string;
  description: string;
  status: 'PENDING' | 'IN_PROGRESS' | 'COMPLETED';
  priority: 'LOW' | 'MEDIUM' | 'HIGH';
  assigned_to: string;
  created_at: string;
}

export interface AccessLog {
  id: number;
  resident_id: number;
  resident_name?: string;
  apartment?: string;
  device_id: string;
  floor: number;
  result: string;
  timestamp: string;
}

export interface SecurityAlert {
  id: number;
  device_id: string;
  reason: string;
  timestamp: string;
  resolved: number;
}

export interface DeviceStatus {
  mode: 'RECOGNIZING' | 'ENROLLING';
  enrollProgress: number;
  elevatorState: string;
  currentFloor: number;
  targetFloor: number;
  error?: string;
}
