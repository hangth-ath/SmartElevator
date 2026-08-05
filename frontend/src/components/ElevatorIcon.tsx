import React from 'react';

export const ElevatorIcon: React.FC<{ size?: number; color?: string; className?: string }> = ({
  size = 20,
  color = 'currentColor',
  className = ''
}) => (
  <svg
    width={size}
    height={size}
    viewBox="0 0 24 24"
    fill="none"
    stroke={color}
    strokeWidth="2"
    strokeLinecap="round"
    strokeLinejoin="round"
    className={className}
  >
    <rect x="3" y="3" width="18" height="18" rx="2" />
    <line x1="12" y1="3" x2="12" y2="21" />
    <path d="M7 10l2-2 2 2" />
    <path d="M17 14l-2 2-2-2" />
  </svg>
);
