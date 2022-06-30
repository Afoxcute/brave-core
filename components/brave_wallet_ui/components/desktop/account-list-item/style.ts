
// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import styled from 'styled-components'
import FlashdriveIcon from '../../../assets/svg-icons/flashdrive-icon.svg'
import { WalletButton } from '../../shared/style'

interface StyleProps {
  orb: string
  icon: string
}

export const StyledWrapper = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-direction: row;
  width: 100%;
  padding: 7px;
  background-color: none;
  border-radius: 10px;
  --show-buttons: none;
  &:hover {
    --show-buttons: flex;
    background-color: ${(p) => p.theme.color.interactive08}15;
  }
`

export const NameAndIcon = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  flex-direction: row;
`

export const AccountAndAddress = styled.div`
  display: flex;
  align-items: flex-start;
  justify-content: center;
  flex-direction: column;
`

export const AccountNameRow = styled.div`
  display: flex;
  align-items: center;
  justify-content: flex-start;
  flex-direction: row;
`

export const AccountName = styled(WalletButton)`
  font-family: Poppins;
  font-size: 13px;
  line-height: 20px;
  letter-spacing: 0.01em;
  font-weight: 600;
  color: ${(p) => p.theme.color.text01};
  cursor: pointer;
  outline: none;
  background: none;
  border: none;
`

export const AccountAddress = styled(WalletButton)`
  font-family: Poppins;
  font-size: 12px;
  line-height: 18px;
  letter-spacing: 0.01em;
  color: ${(p) => p.theme.color.text02};
  cursor: pointer;
  outline: none;
  background: none;
  border: none;
`

export const RightSide = styled.div`
  display: var(--show-buttons);
  align-items: flex-end;
  justify-content: center;
  flex-direction: row;
`

export const AccountCircle = styled.div<Partial<StyleProps>>`
  width: 40px;
  height: 40px;
  border-radius: 100%;
  background-image: url(${(p) => p.orb});
  background-size: cover;
  margin-right: 12px;
`

export const HardwareIcon = styled.div`
  width: 13px;
  height: 13px;
  background-color: ${(p) => p.theme.color.text02};
  -webkit-mask-image: url(${FlashdriveIcon});
  mask-image: url(${FlashdriveIcon});
`

export const Icon = styled.div<Partial<StyleProps>>`
  width: 14px;
  height: 14px;
  margin-left: 4px;
  margin-right: 8px;
  background-color: ${(p) => p.theme.color.interactive07};
  -webkit-mask-image: url(${(p) => p.icon});
  mask-image: url(${(p) => p.icon});
  mask-size: 100%;
`

export const OvalButton = styled(WalletButton)`
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  outline: none;
  background: none;
  border-radius: 48px;
  padding: 3px 10px;
  border: 1px solid ${(p) => p.theme.color.interactive08};
  margin-right: 6px;
`

export const OvalButtonText = styled.span`
  font-family: Poppins;
  font-size: 12px;
  line-height: 18px;
  letter-spacing: 0.01em;
  color: ${(p) => p.theme.color.text02};
  font-weight: 600;
`
