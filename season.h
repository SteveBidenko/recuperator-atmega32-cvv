#ifndef _SEASON_
#define _SEASON_
// ����������� �����������

// �������� �������
extern void cooling_regulator(void);   // ������������ ������������� ���������� (�� �����)
extern void keep_life_in_winter(void); // ��������� ����������������� ������� �����
extern void winter_regulator(void);    // ������������ ������������� ���������� �����
// �������� ���������� ����������
//extern unsigned int time_integration;
void tap_angle_check_range(unsigned char check_mode); // �������� TAP_ANGLE �� �������������� ��������� � ������ ������� �����������. ���� check_mode == INIT_MODE, �������������� PID
#endif
